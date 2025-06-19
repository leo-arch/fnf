#     ____      ____
#    / __/___  / __/
#   / /_/ __ \/ /_  
#  / __/ / / / __/  
# /_/ /_/ /_/_/ key-bindings.zsh
#
# requires fd


file-widget() {
  # print an extra line
  echo ""
  LBUFFER="${LBUFFER}$(fd --hidden | fnf --lines 20 --multi | tr '\n' ' ')"
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
  dir="$(fd --type directory --hidden | fnf --lines 20)"
  [ -d "$dir" ] && cd "$dir"
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
