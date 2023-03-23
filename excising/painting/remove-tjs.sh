#!/usr/bin/env bash
DIR=$(realpath $(dirname ${BASH_SOURCE[0]}))
java -cp "$DIR"/lib/pdfbox-app-3.0.0-RC1.jar "$DIR"/lib/RemoveAllText.java $@
