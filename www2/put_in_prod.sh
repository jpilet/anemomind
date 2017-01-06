#!/bin/bash

set -e 

GIT="git -C dist"

$GIT add .
$GIT commit
$GIT push

#deploy
git -C /var/www/anemolab pull
sudo systemctl restart anemolab@8080
sudo systemctl restart anemolab@8081
