#     ____      ____
#    / __/___  / __/
#   / /_/ __ \/ /_  
#  / __/ / / / __/  
# /_/ /_/ /_/_/ key-bindings.zsh
#
# requires fd

use_find=0

if ! command -v fd; then
  echo "fnf-widget: warning fd is not installed, using find as file searcher"
  use_find=1
fi

file-widget() {
  # print an extra line
  echo ""
  if [ $use_find -eq 0 ]; then
    files=$(fd --hidden)
  else
    files=$(find . -type f)
  fi
  LBUFFER="${LBUFFER}$(echo "$files" | fnf --lines 20 --multi | tr '\n' ' ')"
  # Remove the extra line
  printf "\x1b[1A"
  zle reset-prompt
}
zle -N file-widget
bindkey -M emacs '^t' file-widget
bindkey -M vicmd '^t' file-widget
bindkey -M viins '^t' file-widget

dir-widget() {
  echo ""
  if [ $use_find -eq 0 ]; then
    dirs="$(fd --type directory --hidden)"
  else
    dirs="$(find . -type d )"
  fi
  selected="$(echo $dirs | fnf --lines 20)"
  [ -d "$selected" ] && cd "$selected"
  printf "\x1b[1A"
  zle reset-prompt
}
zle -N dir-widget
bindkey -M emacs '^[c' dir-widget
bindkey -M vicmd '^[c' dir-widget
bindkey -M viins '^[c' dir-widget

history-widget() {
  echo ""
  selected=$(fc -l | fnf --query="$LBUFFER" --reverse | cut -f6- -d' ' )
  [ -n "$selected" ] && LBUFFER="$selected"
  printf "\x1b[1A"
  zle reset-prompt
}
zle -N history-widget
bindkey -M emacs '^r' history-widget
bindkey -M vicmd '^r' history-widget
bindkey -M viins '^r' history-widget
