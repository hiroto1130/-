#ifndef RESULT_H
#define RESULT_H

enum class Result
{
	Success, // 成功
	FailsCreateVertexBuffer,
	FailsCreateIndexBuffer,
	FailsCreatInporter,
	FailsCreatScene,
	FailsInporterInitialize,
	FailsInporter,
	FailsGetNode,
	FailsLoadTextrue,
	NotFoundFileTexture,
	NotFoundMaterial

};

#endif
