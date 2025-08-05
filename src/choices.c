/* choices.c */

/*
 * This file is part of fnf
 * 
 * Copyright
 * (C) 2014-2022 John Hawthorn <john.hawthorn@gmail.com>
 * (C) 2022-2025, L. Abramovich <leo.clifm@outlook.com>
 * All rights reserved.

* The MIT License (MIT)

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "options.h"
#include "choices.h"
#include "match.h"

/* Initial size of buffer for storing input in memory */
#define INITIAL_BUFFER_CAPACITY 4096

/* Initial size of choices array */
#define INITIAL_CHOICE_CAPACITY 128

struct result_list {
	struct scored_result *list;
	size_t size;
};

struct search_job {
	pthread_mutex_t lock;
	choices_t *choices;
	const char *search;
	size_t processed;
	struct worker *workers;
};

struct worker {
	pthread_t thread_id;
	struct search_job *job;
	size_t worker_num;
	unsigned int sort;
	struct result_list result;
};

static int
cmpchoice(const void *idx1, const void *idx2)
{
	const struct scored_result *a = idx1;
	const struct scored_result *b = idx2;

	if (a->score == b->score) {
		/* To ensure a stable sort, we must also sort by the string
		 * pointers. We can do this since we know all the strings are
		 * from a contiguous memory segment (buffer in choices_t). */
		if (a->str < b->str)
			return -1;
		else
			return 1;
	} else if (a->score < b->score) {
		return 1;
	} else {
		return -1;
	}
}

static void *
safe_realloc(void *buffer, const size_t size)
{
	buffer = realloc(buffer, size);
	if (!buffer) {
		fprintf(stderr, "Error: Cannot allocate memory (%zu bytes)\n", size);
		abort();
	}

	return buffer;
}

static void
choices_reset_search(choices_t *c)
{
	free(c->results);
	c->selection = c->available = 0;
	c->results = NULL;
}

static void
choices_resize(choices_t *c, const size_t new_capacity)
{
	c->strings = safe_realloc(c->strings, new_capacity * sizeof(const char *));
	c->capacity = new_capacity;
}

static void
choices_add(choices_t *c, char *choice)
{
	/* Previous search is now invalid */
	choices_reset_search(c);

	if (c->size == c->capacity)
		choices_resize(c, c->capacity * 2);

	c->strings[c->size++] = choice;
}

void
choices_fread(choices_t *c, FILE *file, const char input_delimiter,
	const int max_choices)
{
	/* Save current position for parsing later */
	const size_t buffer_start = c->buffer_size;

	/* Resize buffer to at least one byte more capacity than our current
	 * size. This uses a power of two of INITIAL_BUFFER_CAPACITY.
	 * This must work even when c->buffer is NULL and c->buffer_size is 0. */
	size_t capacity = INITIAL_BUFFER_CAPACITY;
	while (capacity <= c->buffer_size)
		capacity *= 2;
	c->buffer = safe_realloc(c->buffer, capacity);

	/* Continue reading until we get a "short" read, indicating EOF. */
	while ((c->buffer_size += fread(c->buffer + c->buffer_size, 1,
	capacity - c->buffer_size, file)) == capacity) {
		capacity *= 2;
		c->buffer = safe_realloc(c->buffer, capacity);
	}
	c->buffer = safe_realloc(c->buffer, c->buffer_size + 1);
	c->buffer[c->buffer_size++] = '\0';

	/* Truncate buffer to used size, (maybe) freeing some memory for
	 * future allocations. */

	int choices_count = 0;
	/* Tokenize input and add to choices. */
	const char *line_end = c->buffer + c->buffer_size;
	char *line = c->buffer + buffer_start;

	do {
		char *delim = strchr(line, input_delimiter);
		if (delim)
			*delim++ = '\0';

		/* Skip empty lines. */
		if (*line) {
			if (max_choices != -1 && ++choices_count > max_choices)
				break;
			choices_add(c, line);
		}

		line = delim;
	} while (line && line < line_end);
}

void
choices_init(choices_t *c, const options_t *options)
{
	c->strings = NULL;
	c->results = NULL;

	c->buffer_size = 0;
	c->buffer = NULL;

	c->capacity = c->size = 0;
	choices_resize(c, INITIAL_CHOICE_CAPACITY);

	if (options->workers) {
		c->worker_count = options->workers;
	} else {
		const long n = sysconf(_SC_NPROCESSORS_ONLN);
		c->worker_count = n == -1 ? 1 : (size_t)n;
	}

	choices_reset_search(c);
}

void
choices_destroy(choices_t *c)
{
	free(c->buffer);
	c->buffer = NULL;
	c->buffer_size = 0;

	free(c->strings);
	c->strings = NULL;
	c->capacity = c->size = 0;

	free(c->results);
	c->results = NULL;
	c->available = c->selection = 0;
}

size_t
choices_available(const choices_t *c)
{
	return c->available;
}

#define BATCH_SIZE 512
static void
worker_get_next_batch(struct search_job *job, size_t *start, size_t *end)
{
	pthread_mutex_lock(&job->lock);

	*start = job->processed;

	job->processed += BATCH_SIZE;
	if (job->processed > job->choices->size)
		job->processed = job->choices->size;

	*end = job->processed;

	pthread_mutex_unlock(&job->lock);
}
#undef BATCH_SIZE

static struct result_list
merge_result(struct result_list list1, struct result_list list2)
{
	size_t result_index = 0, index1 = 0, index2 = 0;

	struct result_list result;
	result.size = list1.size + list2.size;
	result.list = malloc(result.size * sizeof(struct scored_result));
	if (!result.list) {
		fprintf(stderr, "Error: Cannot allocate memory\n");
		abort();
	}

	while (index1 < list1.size && index2 < list2.size) {
		if (cmpchoice(&list1.list[index1], &list2.list[index2]) < 0)
			result.list[result_index++] = list1.list[index1++];
		else
			result.list[result_index++] = list2.list[index2++];
	}

	while (index1 < list1.size)
		result.list[result_index++] = list1.list[index1++];

	while (index2 < list2.size)
		result.list[result_index++] = list2.list[index2++];

	free(list1.list);
	free(list2.list);

	return result;
}

static void *
choices_search_worker(void *data)
{
	struct worker *w = (struct worker *)data;
	struct search_job *job = w->job;
	const choices_t *c = job->choices;
	struct result_list *result = &w->result;

	size_t start, end;

	for (;;) {
		worker_get_next_batch(job, &start, &end);

		if (start == end)
			break;

		for (size_t i = start; i < end; i++) {
			if (has_match(job->search, c->strings[i])) {
				result->list[result->size].str = c->strings[i];
				result->list[result->size].score = match(job->search, c->strings[i]);
				result->size++;
			}
		}
	}

	/* Sort the partial result */
	if (w->sort == 1)
		qsort(result->list, result->size, sizeof(struct scored_result), cmpchoice);

	/* Fan-in, merging results */
	for (unsigned int step = 0;; step++) {
		if (w->worker_num % (2 << step))
			break;

		unsigned int next_worker = w->worker_num | (1 << step);
		if (next_worker >= c->worker_count)
			break;

		if ((errno = pthread_join(job->workers[next_worker].thread_id, NULL))) {
			perror("pthread_join");
			exit(EXIT_FAILURE);
		}

		w->result = merge_result(w->result, job->workers[next_worker].result);
	}

	return (char *)NULL;
}

void
choices_search(choices_t *c, const char *search, const unsigned int sort)
{
	choices_reset_search(c);

	struct search_job *job = calloc(1, sizeof(struct search_job));
	if (!job) {
		fprintf(stderr, "Error: Cannot allocate memory\n");
		abort();
	}

	job->search = search;
	job->choices = c;
	if (pthread_mutex_init(&job->lock, NULL) != 0) {
		fprintf(stderr, "Error: pthread_mutex_init failed\n");
		abort();
	}

	job->workers = calloc(c->worker_count, sizeof(struct worker));
	if (!job->workers) {
		fprintf(stderr, "Error: Cannot allocate memory\n");
		abort();
	}

	struct worker *workers = job->workers;
	for (int i = (int)c->worker_count - 1; i >= 0; i--) {
		workers[i].job = job;
		workers[i].worker_num = i;
		workers[i].result.size = 0;
		workers[i].sort = sort;
		/* FIXME: This is overkill */
		workers[i].result.list = malloc(c->size * sizeof(struct scored_result));

		/* These must be created last-to-first to avoid a race condition when
		 * fanning in. */
		if ((errno = pthread_create(&workers[i].thread_id, NULL,
		&choices_search_worker, &workers[i]))) {
			perror("pthread_create");
			exit(EXIT_FAILURE);
		}
	}

	if (pthread_join(workers[0].thread_id, NULL)) {
		perror("pthread_join");
		exit(EXIT_FAILURE);
	}

	c->results = workers[0].result.list;
	c->available = workers[0].result.size;

	free(workers);
	pthread_mutex_destroy(&job->lock);
	free(job);
}

const char *
choices_get(const choices_t *c, const size_t n)
{
	if (n < c->available)
		return c->results[n].str;
	return (char *)NULL;
}

score_t
choices_getscore(const choices_t *c, const size_t n)
{
	return c->results[n].score;
}

void
choices_prev(choices_t *c)
{
	if (c->available)
		c->selection = (c->selection + c->available - 1) % c->available;
}

void
choices_next(choices_t *c)
{
	if (c->available)
		c->selection = (c->selection + 1) % c->available;
}
