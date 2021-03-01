#!/bin/bash

qemu-system-aarch64 -machine virt -cpu cortex-a57 -display none -serial stdio -m 32M -kernel ./build/uk_test_suite_kvm-arm64 -append "test.test_var=-30 test.test_var1=40 test.test_var2 = 1024 test.test_var3=-1024 test.test_var4=65536 test.test_var5=-65536 test.test_var6= 128 test.test_var7 =\"BC\" test.testString=\"Hello TestSuite\" test.testarr1=\"1 2 3\" test.testarr2=\"1 2 3 4 5\" test.testarr3=\"1 2\" --
