struct P_INPUT
{
	float4 posH : SV_POSITION;
	float4 colorH : COLOR;
};

float4 main(P_INPUT _input) : SV_TARGET
{
	return _input.colorH;
}