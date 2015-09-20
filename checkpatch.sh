git diff 48b91ec5356d8c4baf948247deb2208e005c996d -- '*.c' '*.h' '*.S' > srcdiff
perl checkpatch.pl --ignore FILE_PATH_CHANGES -terse --no-signoff -no-tree srcdiff