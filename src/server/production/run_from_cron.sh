#!/bin/bash
set -e


env | grep -v "=$" | grep -v "_=" > /etc/cron.d/anemo-cron

echo "*/3    *    *   *   *    flock -n /tmp/process_logs.lock /anemomind/bin/processNewLogs_DockerDev.sh >> /var/log/cron.log 2>&1" >> /etc/cron.d/anemo-cron
chmod 0644 /etc/cron.d/anemo-cron

# Apply cron job
crontab /etc/cron.d/anemo-cron

# Create the log file to be able to run tail
touch /var/log/cron.log

# Run the command on container startup
cron

tail -f /var/log/cron.log
