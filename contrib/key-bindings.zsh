#     ____      ____
#    / __/___  / __/
#   / /_/ __ \/ /_  
#  / __/ / / / __/  
# /_/ /_/ /_/_/ key-bindings.zsh
#
# requires fd

# TODO: make fnf appear on next line instead of replacing prompt and line buffer
file-widget() {
	LBUFFER="${LBUFFER}$(fd --hidden | fnf --lines 20 --multi | tr '\n' ' ')"
	zle reset-prompt
}
zle -N file-widget
bindkey -M emacs '^t' file-widget
bindkey -M vicmd '^t' file-widget
bindkey -M viins '^t' file-widget

dir-widget() {
	dir="$(fd --type directory --hidden | fnf --lines 20)"
	[ -d "$dir" ] && cd "$dir"
	zle reset-prompt
}
zle -N dir-widget
bindkey -M emacs '^[c' dir-widget
bindkey -M vicmd '^[c' dir-widget
bindkey -M viins '^[c' dir-widget

history-widget() {
	selected=$(fc -l | fnf --query="$LBUFFER" --reverse | cut -f6- -d' ' )
	[ -n "$selected" ] && LBUFFER="$selected"
	zle reset-prompt
}
zle -N history-widget
bindkey -M emacs '^r' history-widget
bindkey -M vicmd '^r' history-widget
bindkey -M viins '^r' history-widget
