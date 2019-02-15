# Testing

## Unit Tests

We use the Catch-2 test framework to write the unit tests.  The unit tests test the internal logic
and mock out physical hardware access with dummy stubs that we can check were called.  

The units test are written for the following components:
 - codec
 - buddy host library

 Future:
  - firmware
  - python bindings
  - any future language binding

The unit tests are configured to run with the CI and a coverage report is generated and reported on
coveralls hosted site.  Future plan for manual HTML generation are being made.  