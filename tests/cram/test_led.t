test that led is providing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ valgrind --quiet --leak-check=full test-led 2>&1 | head -n 30
  test-led: led_run: adding foo:green:wps delta=25 timeout=100 brightness=255 original=0 blink=0 fade=1 on=1000 off=1000
  test-led: led_run: adding foo:red:error delta=51 timeout=100 brightness=0 original=255 blink=0 fade=1 on=500 off=500
  test-led: led_set: set foo:green:wps to 25
  test-led: led_set: set foo:red:error to 255
  test-led: led_set: set foo:green:wps to 50
  test-led: led_set: set foo:red:error to 204
  test-led: led_set: set foo:green:wps to 75
  test-led: led_set: set foo:red:error to 153
  test-led: led_set: set foo:green:wps to 100
  test-led: led_set: set foo:red:error to 102
  test-led: led_set: set foo:green:wps to 125
  test-led: led_set: set foo:red:error to 51
  test-led: led_set: set foo:green:wps to 150
  test-led: led_set: set foo:red:error to 0
  test-led: led_state_set: foo:red:error to LED_FADE_IN
  test-led: led_set: set foo:green:wps to 175
  test-led: led_set: set foo:red:error to 51
  test-led: led_set: set foo:green:wps to 200
  test-led: led_set: set foo:red:error to 102
  test-led: led_set: set foo:green:wps to 225
  test-led: led_set: set foo:red:error to 153
  test-led: led_set: set foo:green:wps to 250
  test-led: led_set: set foo:red:error to 204
  test-led: led_set: set foo:green:wps to 255
  test-led: led_state_set: foo:green:wps to LED_FADE_OUT
  test-led: led_set: set foo:red:error to 255
  test-led: led_state_set: foo:red:error to LED_FADE_OUT
  test-led: led_set: set foo:green:wps to 230
  test-led: led_set: set foo:red:error to 204
  test-led: led_set: set foo:green:wps to 205
