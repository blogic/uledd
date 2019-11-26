test that blob-led is providing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export TEST_INPUTS="$TESTDIR/inputs"
  $ alias tb="valgrind --quiet --leak-check=full test-blob-led"

  $ tb $TEST_INPUTS/scene-json-default/wps.json
  test-blob-led: ap:green:status brightness=255 original=0 blink=0 fade=1 on=2000 off=2000
  test-blob-led: ap:red:status brightness=0 original=255 blink=0 fade=1 on=2000 off=2000
