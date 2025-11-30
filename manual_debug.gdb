set logging overwrite on
set logging enabled
set pagination off
set confirm off

tbreak main
continue

set $syscall = 0x8
set $stimer = 0x8000000000000005
set $page_fault = 0xc
set $__exit_addr = 0x20000094

break IC::entry if $scause != $syscall && $scause != $stimer && !($scause == $page_fault && $sepc == $__exit_addr)
break Machine::reboot

command 3
    kill
    quit
end

continue

