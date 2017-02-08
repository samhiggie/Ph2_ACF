#!/bin/bash
SESSION_NAME=irradtest
BASE_DIR=$(pwd)

tmux list-session 2>&1 | grep -q "^$SESSION_NAME" || tmux new-session -s $SESSION_NAME -d
tmux send-keys -t $SESSION_NAME "cd $BASE_DIR" Enter

tmux send-keys -t $SESSION_NAME ". $BASE_DIR/setup.sh" Enter
tmux send-keys -t $SESSION_NAME "echo \'Running cbc3irradiation test\'" Enter
#tmux send-keys -t $SESSION_NAME C-m
tmux send-keys -t $SESSION_NAME "while true; do
    $BASE_DIR/bin/cbc3irrad -b | tee $BASE_DIR/consoledump.log
    echo \'bin/cbc3irrad finished with exit code $?. Respawning...\' | tee $BASE_DIR/consoledump.log
    sleep 10
done" Enter
#tmux send-keys -t $SESSION_NAME C-m
