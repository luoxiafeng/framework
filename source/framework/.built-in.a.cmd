cmd_drivers/framework/built-in.a := rm -f drivers/framework/built-in.a;  printf "drivers/framework/%s " task/built-in.a | xargs /home/rlk/xiafeng.luo/CrossCompileTools/gcc-linaro-12.2.1-2022.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-ar cDPrST drivers/framework/built-in.a
