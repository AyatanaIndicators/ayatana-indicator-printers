#!/bin/sh

which mate-autogen || {
    echo "Could not find 'mate-autgen'. Is mate-common installed?"
    exit 1
}

. mate-autogen

