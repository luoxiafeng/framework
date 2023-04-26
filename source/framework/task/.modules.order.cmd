cmd_drivers/framework/task/modules.order := {   echo drivers/framework/task/dump_segments.ko; :; } | awk '!x[$$0]++' - > drivers/framework/task/modules.order
