# Script to set up bash/zsh environment with path to Eldr executable
# Needs to be sourced

if [[ "$#" -ge "1" ]]; then
    BUILD_DIR="$1"
else
    BUILD_DIR="build"
fi

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "Usage:\n"
    echo "$ source setpath.sh\n"
    exit 0
fi

if [ "$BASH_VERSION" ]; then
    ELDR_DIR=$(dirname "$BASH_SOURCE")
    export ELDR_DIR=$(builtin cd "$ELDR_DIR"; builtin pwd)
elif [ "$ZSH_VERSION" ]; then
    export ELDR_DIR=$(dirname "$0:A")
fi

export PATH="$ELDR_DIR/$BUILD_DIR/src/eldr:$PATH"
