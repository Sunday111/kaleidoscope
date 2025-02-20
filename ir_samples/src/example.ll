; ModuleID = '/home/sunday/kaleidoscope-compiler/ir_samples/src/simple_function.cpp'
source_filename = "/home/sunday/kaleidoscope-compiler/ir_samples/src/simple_function.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i32:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$_Z1giii = comdat any

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: mustprogress noinline norecurse optnone uwtable
define dso_local noundef i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %2 = call noundef i32 @_Z1giii()
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2)
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define linkonce_odr dso_local noundef i32 @_Z1giii() #2 comdat {
  %1 = alloca i32, align 8
  store i32 42, ptr %1, align 8
  %2 = load i32, ptr %1, align 8
  %3 = alloca i32, align 8
  store i32 21, ptr %3, align 8
  %4 = load i32, ptr %3, align 8
  %5 = sub i32 %2, %4
  ret i32 %5
}

attributes #0 = { mustprogress noinline norecurse optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 20.0.0git (https://github.com/llvm/llvm-project 4a6fcc17c6d5ab9bf2b9629b49fce607a4eb4e25)"}
