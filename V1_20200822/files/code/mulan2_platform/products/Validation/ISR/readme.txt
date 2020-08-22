TEST DESCRIPTION:
   This validation test target is interrupt controller issue detection.
   Due to this issue XIn_PEND register could be cleaned by another External Interrupt source.
   If this test FAIL, then two External Interrupt sources couldn't be resolved in a safe way.

Validation test is designed for 813xx product line