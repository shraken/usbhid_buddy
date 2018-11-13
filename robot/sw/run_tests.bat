call robot -T -d reports adc_dynamic.robot
timeout /t 5
call robot -T -d reports adc_static.robot
timeout /t 5
call robot -T -d reports dac_dynamic.robot
timeout /t 5
call robot -T -d reports dac_static.robot
timeout /t 5
call robot -T -d reports pwm_duty.robot
timeout /t 5
call robot -T -d reports pwm_frequency.robot
timeout /t 5
call robot -T -d reports counter.robot