SET JLINK_OPTS=-device EFM8UB20F64G -if c2 -speed 200
SET JLINK=JLink %JLINK_OPTS%

%JLINK% flash.jlink