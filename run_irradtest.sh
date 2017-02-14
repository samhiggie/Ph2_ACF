#!/bin/bash
export SESSION_NAME=irradtest
#direcories
export TMUX_BASE_DIR=$(pwd)
export TMUX_USB_DIR=$TMUX_BASE_DIR/../Ph2_USBInstDriver
#tmux panes
export HMP_PANE=1
export KE_PANE=2
export MAIN_PANE=0
#HMP ports
export HMP_HTTP_PORT=8080
export HMP_ZMQ_PORT=8081
#KE ports
export KE_HTTP_PORT=8082
export KE_ZMQ_PORT=8083
#monitoring interval
export INTERVAL=2
#export CYCLECOUNT=1

#params: 1 pane, 2 working dir
function setup_env {
    tmux select-window -t $SESSION_NAME
    tmux select-pane -t $1
    tmux send-keys -t $SESSION_NAME "cd $2" C-m
    tmux send-keys -t $SESSION_NAME ". $2/setup.sh" C-m
}

function start_HMP {
        tmux select-pane -t $HMP_PANE
        tmux send-keys  -t $SESSION_NAME C-c
        tmux send-keys -t $SESSION_NAME "cd $TMUX_USB_DIR" C-m
        tmux send-keys -t $SESSION_NAME ". $TMUX_USB_DIR/setup.sh" C-m
        tmux send-keys -t $SESSION_NAME "lvSupervisor -f $TMUX_USB_DIR/settings/HMP4040.xml -r $HMP_ZMQ_PORT -p $HMP_HTTP_PORT -i $INTERVAL -S" C-m
}

function start_KE {
        tmux select-window -t $SESSION_NAME
        tmux select-pane -t $KE_PANE
        tmux send-keys -t $SESSION_NAME C-c
        tmux send-keys -t $SESSION_NAME "cd $TMUX_USB_DIR" C-m
        tmux send-keys -t $SESSION_NAME ". $TMUX_USB_DIR/setup.sh" C-m
        tmux send-keys -t $SESSION_NAME "dmmSupervisor -r $KE_ZMQ_PORT -p $KE_HTTP_PORT -i $INTERVAL" C-m
}

export -f start_HMP
export -f start_KE


function run_irradtest {
    #CYCLECOUNT=1;
    tmux select-window -t $SESSION_NAME
    tmux select-pane -t $MAIN_PANE
    tmux send-keys -t $SESSION_NAME "
    while true; do
        $TMUX_BASE_DIR/bin/cbc3irrad -s -b | tee $TMUX_BASE_DIR/consoledump.log
        echo \'bin/cbc3irrad finished with exit code $?. Respawning...\' | tee $TMUX_BASE_DIR/consoledump.log
        #if (("$CYCLECOUNT" % 1 == 0)); then 
            #echo \'Re-starting monitoring servers\'
            #start_HMP
            #start_KE
        #fi
        sleep 20
    done" C-m
}

#default pane (left) is 0
#upper right is pane 1
#lower right is pane 2
tmux list-session 2>&1 | grep -q "^$SESSION_NAME" || tmux new-session -s $SESSION_NAME -d
tmux rename-window 'overview'
#check if 3 panes already exist
npanes=$(tmux list-panes | wc -l)
if (($npanes == 1)); then
    tmux split-window -h 
    tmux split-window -v
elif (($npanes == 3)); then
    echo "enough panes available!"
fi

# cd to Ph2_ACF directory in left pane and start Monitoring servers in the right panes 1/2
setup_env 0 $TMUX_BASE_DIR
start_HMP 
start_KE
run_irradtest

#tmux send-keys -t $SESSION_NAME ". $TMUX_BASE_DIR/setup.sh" Enter
#tmux send-keys -t $SESSION_NAME "echo \'Running cbc3irradiation test\'" Enter
##tmux send-keys -t $SESSION_NAME C-m
#tmux send-keys -t $SESSION_NAME "while true; do
    #$TMUX_BASE_DIR/bin/cbc3irrad -b | tee $TMUX_BASE_DIR/consoledump.log
    #echo \'bin/cbc3irrad finished with exit code $?. Respawning...\' | tee $TMUX_BASE_DIR/consoledump.log
    #sleep 10
#done" Enter
#tmux send-keys -t $SESSION_NAME C-m
