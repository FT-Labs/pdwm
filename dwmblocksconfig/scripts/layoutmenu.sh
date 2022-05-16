#!/bin/sh

cat <<EOF | xmenu
[]=   Tiled Layout	0
TTT   Backstack Layout	1
[M]   Monocle Layout		2
H[]   Deck Layout		3
[@]   Spiral Layout	4
[\\]   Dwindle Layout		5
|M|   Centered Master Layout		6
>M>   Centered Floating Master Layout		7
><>   Floating Layout		8
EOF
