setup test environment:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export TEST_INPUTS="$TESTDIR/inputs"
  $ alias scene-basic="valgrind --quiet --leak-check=full test-scene-basic"

test that scene-basic is providing expected results:

  $ scene-basic $TEST_INPUTS/scene-json-default 2>&1 | sed "s;$TESTDIR;;"
  test-scene-json: scene_json_load_file: loading /inputs/scene-json-default/backdoor.json
  test-scene-json: scene_add: add backdoor priority=5 timeout=2000 state=SCENE_IDLE
  test-scene-json: led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-json: scene_led_add: ap:orange:sms brightness=128 original=0 blink=0 fade=1 on=500 off=500
  test-scene-json: scene_json_load_file: loading /inputs/scene-json-default/broken.json
  test-scene-json: failed to parse /inputs/scene-json-default/broken.json
  test-scene-json: scene_json_load_file: loading /inputs/scene-json-default/short.json
  test-scene-json: scene_add: add short priority=1 timeout=1500 state=SCENE_IDLE
  test-scene-json: led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-json: scene_led_add: ap:purple:short brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-json: scene_json_load_file: loading /inputs/scene-json-default/sms.json
  test-scene-json: scene_add: add sms priority=6 timeout=1500 state=SCENE_IDLE
  test-scene-json: led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-json: scene_led_add: ap:orange:sms brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-json: scene_json_load_file: loading /inputs/scene-json-default/wps.json
  test-scene-json: scene_add: add wps priority=3 timeout=35000 state=SCENE_IDLE
  test-scene-json: led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-json: scene_led_add: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-scene-json: led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-json: scene_led_add: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
  test-scene-json: led_run: ap:purple:short delta=25 timeout=100 brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-scene-json: set_scene_state: short to SCENE_RUNNING
  test-scene-json: led_set: set ap:purple:short to 25
  test-scene-json: led_set: set ap:purple:short to 50
  test-scene-json: led_set: set ap:purple:short to 75
  test-scene-json: led_set: set ap:purple:short to 100
  test-scene-json: led_set: set ap:purple:short to 125
  test-scene-json: led_set: set ap:purple:short to 150
  test-scene-json: led_set: set ap:purple:short to 175
  test-scene-json: led_set: set ap:purple:short to 200
  test-scene-json: led_set: set ap:purple:short to 225
  test-scene-json: led_set: set ap:purple:short to 250
  test-scene-json: led_set: set ap:purple:short to 255
  test-scene-json: led_state_set: ap:purple:short to LED_FADE_OUT
  test-scene-json: led_set: set ap:purple:short to 230
  test-scene-json: led_set: set ap:purple:short to 205
  test-scene-json: led_set: set ap:purple:short to 180
  test-scene-json: set_scene_state: short to SCENE_IDLE
  test-scene-json: finished!
