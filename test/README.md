Instructions on how to execute UTEST.

* When using an SDK - make sure it has been built
  for an x84 machine.

- First build with a call to ./bootstrap.sh
- Once built, configure flags with
  "./configure ${CONFIGURE_FLAGS} --enable-oe-sdk"
- Be sure to include --enable-oe-sdk or the tests
  will not run properly.
- Lastly "make check" will create the executable
  utest.

* WHEN RUNNING UTEST remember to take advantage
  of the gtest capabilities. "./utest --help"
  - --gtest_repeat=[COUNT]
  - --gtest_shuffle
  - --gtest_random_seed=[NUMBER]
