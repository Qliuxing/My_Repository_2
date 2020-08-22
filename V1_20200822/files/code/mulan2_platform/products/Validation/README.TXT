General validation tests
------------------------
build_compilation_test.bat 
clean_compilation_test.bat 
    Used to build (and clean respectively) compilation tests.
    For each product within the SW Platform:
        - build all product's profiles from \products\<product>\config\*.mk
        - build all product's examples from \products\<product>\examples

build_validation_tests.bat 
clean_validation_tests.bat 
    Used to build (and clean respectively) validation test examples:
     __________________
    /- Product 81300 -/
    - LIN_SAE_FIXED_DNN
    - LIN_SAE_SOFTWARE_DNN
    - LIN13
    - LIN20_Event_Triggered
    - LIN21_Signal_layer
    - MATHLIB
    - ISR
     __________________
    /- Product 81150 -/
    - LIN_SAE_FIXED_DNN
    - LIN_SAE_SOFTWARE_DNN
    - LIN20_Event_Triggered

LIN Conformance Tests (at IHR Lab)
----------------------------------
Applications below are supposed to be used for the LIN Conformance Tests (CT) at IHR Lab

LIN1.3 CT:
        mulan2_platform\products\Validation\LIN13

LIN2.0 CT:
        mulan2_platform\products\Validation\LIN20_Event_Triggered

LIN2.1 CT
        mulan2_platform\products\Validation\LIN21_Signal_layer

SAE J2602 CT:
        mulan2_platform\products\Validation\LIN_SAE_FIXED_DNN
        mulan2_platform\products\Validation\LIN_SAE_SOFTWARE_DNN 
