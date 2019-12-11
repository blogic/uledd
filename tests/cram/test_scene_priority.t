setup test environment:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export TEST_INPUTS="$TESTDIR/inputs"
  $ alias scene-priority="valgrind --quiet --leak-check=full test-scene-priority"

test that sms has higher priority and runs over backdoor:

  $ scene-priority $TEST_INPUTS/scene-json-default backdoor sms 2>&1 | sed "s;$TESTDIR;;"
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/backdoor.json
  test-scene-priority: scene_add: add backdoor priority=5 timeout=2000 state=SCENE_IDLE
  test-scene-priority: led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-priority: scene_led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/broken.json
  test-scene-priority: failed to parse /inputs/scene-json-default/broken.json
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/short.json
  test-scene-priority: scene_add: add short priority=1 timeout=1500 state=SCENE_IDLE
  test-scene-priority: led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/sms.json
  test-scene-priority: scene_add: add sms priority=6 timeout=1500 state=SCENE_IDLE
  test-scene-priority: led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/wps.json
  test-scene-priority: scene_add: add wps priority=3 timeout=35000 state=SCENE_IDLE
  test-scene-priority: led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: scene_led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: scene_led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: led_run: ap:orange:sms delta=25 timeout=100 brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: set_scene_state: backdoor to SCENE_RUNNING
  test-scene-priority: led_set: set ap:orange:sms to 25
  test-scene-priority: led_set: set ap:orange:sms to 50
  test-scene-priority: set_scene_state: backdoor to SCENE_PAUSED
  test-scene-priority: led_run: ap:orange:sms delta=25 timeout=100 brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: set_scene_state: sms to SCENE_RUNNING
  test-scene-priority: led_set: set ap:orange:sms to 75
  test-scene-priority: led_set: set ap:orange:sms to 100
  test-scene-priority: led_set: set ap:orange:sms to 125
  test-scene-priority: led_set: set ap:orange:sms to 150
  test-scene-priority: led_set: set ap:orange:sms to 175
  test-scene-priority: led_set: set ap:orange:sms to 200
  test-scene-priority: led_set: set ap:orange:sms to 225
  test-scene-priority: led_set: set ap:orange:sms to 250
  test-scene-priority: led_set: set ap:orange:sms to 255
  test-scene-priority: led_state_set: ap:orange:sms to LED_FADE_OUT
  test-scene-priority: led_set: set ap:orange:sms to 230
  test-scene-priority: led_set: set ap:orange:sms to 205
  test-scene-priority: led_set: set ap:orange:sms to 180
  test-scene-priority: led_set: set ap:orange:sms to 155
  test-scene-priority: led_set: set ap:orange:sms to 130
  test-scene-priority: set_scene_state: sms to SCENE_IDLE
  test-scene-priority: led_run: ap:orange:sms delta=25 timeout=100 brightness=0 original=255 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: set_scene_state: backdoor to SCENE_RUNNING
  test-scene-priority: finished!

test that sms has higher priority and backdoor cant interrupt it:

  $ scene-priority $TEST_INPUTS/scene-json-default sms backdoor 2>&1 | sed "s;$TESTDIR;;"
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/backdoor.json
  test-scene-priority: scene_add: add backdoor priority=5 timeout=2000 state=SCENE_IDLE
  test-scene-priority: led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-priority: scene_led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/broken.json
  test-scene-priority: failed to parse /inputs/scene-json-default/broken.json
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/short.json
  test-scene-priority: scene_add: add short priority=1 timeout=1500 state=SCENE_IDLE
  test-scene-priority: led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/sms.json
  test-scene-priority: scene_add: add sms priority=6 timeout=1500 state=SCENE_IDLE
  test-scene-priority: led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: scene_json_load_file: loading /inputs/scene-json-default/wps.json
  test-scene-priority: scene_add: add wps priority=3 timeout=35000 state=SCENE_IDLE
  test-scene-priority: led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: scene_led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: scene_led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-priority: led_run: ap:orange:sms delta=25 timeout=100 brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-priority: set_scene_state: sms to SCENE_RUNNING
  test-scene-priority: led_set: set ap:orange:sms to 25
  test-scene-priority: led_set: set ap:orange:sms to 50
  test-scene-priority: check_scene_priority: not running backdoor (prio=5) < sms (prio=6)
  test-scene-priority: led_set: set ap:orange:sms to 75
  test-scene-priority: led_set: set ap:orange:sms to 100
  test-scene-priority: led_set: set ap:orange:sms to 125
  test-scene-priority: led_set: set ap:orange:sms to 150
  test-scene-priority: led_set: set ap:orange:sms to 175
  test-scene-priority: led_set: set ap:orange:sms to 200
  test-scene-priority: led_set: set ap:orange:sms to 225
  test-scene-priority: led_set: set ap:orange:sms to 250
  test-scene-priority: led_set: set ap:orange:sms to 255
  test-scene-priority: led_state_set: ap:orange:sms to LED_FADE_OUT
  test-scene-priority: led_set: set ap:orange:sms to 230
  test-scene-priority: led_set: set ap:orange:sms to 205
  test-scene-priority: led_set: set ap:orange:sms to 180
  test-scene-priority: set_scene_state: sms to SCENE_IDLE
  test-scene-priority: finished!
