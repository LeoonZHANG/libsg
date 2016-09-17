#!/usr/bin/env bash

if [ "$(uname)" != "Darwin" ]; then
    echo "brew can be installed under macOS only\n" > &2;
elif
    /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
fi