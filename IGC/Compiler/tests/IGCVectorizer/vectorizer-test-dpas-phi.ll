; RUN: igc_opt --igc-vectorizer -S -dce < %s 2>&1 | FileCheck %s

; CHECK: %vectorized_phi = phi <8 x float> [ zeroinitializer, %bb60 ], [ %tmp113, %bb88 ]
; CHECK: %vectorized_phi1 = phi <8 x float> [ zeroinitializer, %bb43 ], [ %tmp113, %bb88 ]

define spir_kernel void @quux() {
bb43:
  br label %bb123

bb60:                                             ; No predecessors!
  br label %bb88

bb88:                                             ; preds = %bb88, %bb60
  %tmp90 = phi float [ 0.000000e+00, %bb60 ], [ %tmp114, %bb88 ]
  %tmp91 = phi float [ 0.000000e+00, %bb60 ], [ %tmp115, %bb88 ]
  %tmp92 = phi float [ 0.000000e+00, %bb60 ], [ %tmp116, %bb88 ]
  %tmp93 = phi float [ 0.000000e+00, %bb60 ], [ %tmp117, %bb88 ]
  %tmp94 = phi float [ 0.000000e+00, %bb60 ], [ %tmp118, %bb88 ]
  %tmp95 = phi float [ 0.000000e+00, %bb60 ], [ %tmp119, %bb88 ]
  %tmp96 = phi float [ 0.000000e+00, %bb60 ], [ %tmp120, %bb88 ]
  %tmp97 = phi float [ 0.000000e+00, %bb60 ], [ %tmp121, %bb88 ]
  %tmp104 = insertelement <8 x float> zeroinitializer, float %tmp90, i64 0
  %tmp105 = insertelement <8 x float> %tmp104, float %tmp91, i64 1
  %tmp106 = insertelement <8 x float> %tmp105, float %tmp92, i64 2
  %tmp107 = insertelement <8 x float> %tmp106, float %tmp93, i64 3
  %tmp108 = insertelement <8 x float> %tmp107, float %tmp94, i64 4
  %tmp109 = insertelement <8 x float> %tmp108, float %tmp95, i64 5
  %tmp110 = insertelement <8 x float> %tmp109, float %tmp96, i64 6
  %tmp111 = insertelement <8 x float> %tmp110, float %tmp97, i64 7
  %tmp112 = call <8 x float> @llvm.genx.GenISA.sub.group.dpas.v8f32.v8f32.v8i16.v8i32(<8 x float> %tmp111, <8 x i16> zeroinitializer, <8 x i32> zeroinitializer, i32 0, i32 0, i32 0, i32 0, i1 false)
  %tmp113 = call <8 x float> @llvm.genx.GenISA.sub.group.dpas.v8f32.v8f32.v8i16.v8i32(<8 x float> zeroinitializer, <8 x i16> zeroinitializer, <8 x i32> zeroinitializer, i32 0, i32 0, i32 0, i32 0, i1 false)
  %tmp114 = extractelement <8 x float> %tmp113, i64 0
  %tmp115 = extractelement <8 x float> %tmp113, i64 1
  %tmp116 = extractelement <8 x float> %tmp113, i64 2
  %tmp117 = extractelement <8 x float> %tmp113, i64 3
  %tmp118 = extractelement <8 x float> %tmp113, i64 4
  %tmp119 = extractelement <8 x float> %tmp113, i64 5
  %tmp120 = extractelement <8 x float> %tmp113, i64 6
  %tmp121 = extractelement <8 x float> %tmp113, i64 7
  br i1 false, label %bb88, label %bb123

bb123:                                            ; preds = %bb88, %bb43
  %tmp133 = phi float [ 0.000000e+00, %bb43 ], [ %tmp114, %bb88 ]
  %tmp134 = phi float [ 0.000000e+00, %bb43 ], [ %tmp115, %bb88 ]
  %tmp135 = phi float [ 0.000000e+00, %bb43 ], [ %tmp116, %bb88 ]
  %tmp136 = phi float [ 0.000000e+00, %bb43 ], [ %tmp117, %bb88 ]
  %tmp137 = phi float [ 0.000000e+00, %bb43 ], [ %tmp118, %bb88 ]
  %tmp138 = phi float [ 0.000000e+00, %bb43 ], [ %tmp119, %bb88 ]
  %tmp139 = phi float [ 0.000000e+00, %bb43 ], [ %tmp120, %bb88 ]
  %tmp140 = phi float [ 0.000000e+00, %bb43 ], [ %tmp121, %bb88 ]
  %tmp143 = insertelement <8 x float> zeroinitializer, float %tmp133, i64 0
  %tmp144 = insertelement <8 x float> %tmp143, float %tmp134, i64 1
  %tmp145 = insertelement <8 x float> %tmp144, float %tmp135, i64 2
  %tmp146 = insertelement <8 x float> %tmp145, float %tmp136, i64 3
  %tmp147 = insertelement <8 x float> %tmp146, float %tmp137, i64 4
  %tmp148 = insertelement <8 x float> %tmp147, float %tmp138, i64 5
  %tmp149 = insertelement <8 x float> %tmp148, float %tmp139, i64 6
  %tmp150 = insertelement <8 x float> %tmp149, float %tmp140, i64 7
  %tmp151 = bitcast <8 x float> %tmp150 to <8 x i32>
  call void @llvm.genx.GenISA.LSC2DBlockWrite.v8i32(i64 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i1 false, i1 false, i32 0, <8 x i32> %tmp151)
  ret void
}

declare <8 x float> @llvm.genx.GenISA.sub.group.dpas.v8f32.v8f32.v8i16.v8i32(<8 x float>, <8 x i16>, <8 x i32>, i32, i32, i32, i32, i1)

declare <8 x i16> @llvm.genx.GenISA.LSC2DBlockRead.v8i16(i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i1, i1, i32)

declare <8 x i32> @llvm.genx.GenISA.LSC2DBlockRead.v8i32(i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i1, i1, i32)

declare void @llvm.genx.GenISA.LSC2DBlockWrite.v8i32(i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i1, i1, i32, <8 x i32>)
