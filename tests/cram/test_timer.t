test that timer is providing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ valgrind --quiet --leak-check=full test-timer
  test-timer: foo1 timer fired 1x
  test-timer: foo2 timer fired 1x
  test-timer: foo3 timer fired 1x
  test-timer: foo1 timer fired 2x
  test-timer: foo2 timer fired 2x
  test-timer: foo1 timer fired 3x
  test-timer: foo3 timer fired 2x
  test-timer: foo2 timer fired 3x
  test-timer: finished!
