#!/bin/bash

# dump environment variables
printenv | sed 's/^\(.*\)$/export \1/g' >> /etc/environment

cron 