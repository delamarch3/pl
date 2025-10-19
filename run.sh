#!/bin/bash

set -e

make > main.b

~/Projects/systems/stack/target/debug/stackc main.b
rm main.b

~/Projects/systems/stack/target/debug/stack a.out
rm a.out
