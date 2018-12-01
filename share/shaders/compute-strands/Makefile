all: ClipCurves.spv DrawStrands.spv PrefixSum_1.spv PrefixSum_2.spv ReduceDepthBuffer.spv Reorder.spv

ClipCurves.spv: ClipCurves.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=ClipCurves ClipCurves.hlsl

DrawStrands.spv: DrawStrands.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=DrawStrands DrawStrands.hlsl

PrefixSum_1.spv: PrefixSum_1.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl ScanBlockBuffer.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=PrefixSum_1 PrefixSum_1.hlsl

PrefixSum_2.spv: PrefixSum_2.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl ScanBlockBuffer.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=PrefixSum_2 PrefixSum_2.hlsl

ReduceDepthBuffer.spv: ReduceDepthBuffer.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=ReduceDepthBuffer ReduceDepthBuffer.hlsl

Reorder.spv: Reorder.hlsl Curves.hlsl Commons.hlsl Random.hlsl Constants.hlsl
	glslc -O -g -c -fshader-stage=compute -fentry-point=Reorder Reorder.hlsl