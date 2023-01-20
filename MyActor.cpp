// Fill out your copyright notice in the Description page of Project Settings.


#include "MyActor.h"

#include "RHIResources.h"
#include "DrawDebugHelpers.h"
#include "Templates/RefCounting.h"
#include "RenderGraphUtils.h"


#include "RayTracingInstanceBufferUtil.h"
#include "RayTracingDefinitions.h"
#include "SceneTypes.h"
#include "Runtime/Renderer/Private/ScenePrivate.h"

#include "RayTracingDynamicGeometryCollection.h"

#include "PixelShaderUtils.h"

#include "Shader.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterUtils.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "RHIUtilities.h"

#include "Containers/DynamicRHIResourceArray.h"
#include "Modules/ModuleManager.h"
#include "Runtime/Renderer/Private/RayTracing/RayTracingScene.h"
#include "ProceduralMeshComponent.h"

class FSimpleVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSimpleVS);
	SHADER_USE_PARAMETER_STRUCT(FSimpleVS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector4f, InPosition)
		SHADER_PARAMETER(FVector4f, Output)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);		
	}
};
IMPLEMENT_SHADER_TYPE(,FSimpleVS, TEXT("/Engine/Private/MyTest.usf"), TEXT("MainVS"), SF_Vertex);

class FSimplePS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSimplePS);
	SHADER_USE_PARAMETER_STRUCT(FSimplePS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
	SHADER_PARAMETER(FVector4f, StartColor)
	RENDER_TARGET_BINDING_SLOTS()

	END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};
IMPLEMENT_SHADER_TYPE(,FSimplePS,TEXT("/Engine/Private/MyTest.usf"), TEXT("MainPS"), SF_Pixel);

BEGIN_SHADER_PARAMETER_STRUCT(FCopyTextureParameters, )

// Declares CopySrc access to an FRDGTexture*
RDG_TEXTURE_ACCESS(Input, ERHIAccess::CopySrc)

RDG_TEXTURE_ACCESS(Output, ERHIAccess::CopyDest)

END_SHADER_PARAMETER_STRUCT()




AMyActor::AMyActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); 
	//ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT(Proc"Mesh"));
	//sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Camera"));
	//ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> RenderTargetAsset(TEXT("/Game/MyRT.MyRT"));
	const FString RenderTargetName = "/Game/MyRT.MyRT";
	RenderTarget2D = LoadObject<UTextureRenderTarget2D>(nullptr, *RenderTargetName);
	check(RenderTarget2D);
	//RenderTarget2D = RenderTargetAsset.Object;
	//Texture2D = RenderTargetAsset.Object->ConstructTexture2D(this, "mytexture", RenderTarget2D->GetFlags(), 255);
	
	//RenderTarget2D = NewObject<UTextureRenderTarget2D>();
	//RenderTarget2D->bNeedsTwoCopies = false;
	//RenderTarget2D->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA32f;
	//RenderTarget2D->bAutoGenerateMips = false;
	//RenderTarget2D->bGPUSharedFlag = true;
	//RenderTarget2D->UpdateResource();
	//RenderTarget2D = NewObject<UTextureRenderTarget2D>();

	VisualMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));

	if (CubeVisualAsset.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeVisualAsset.Object);
		VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}

	// init variables with values
	LocationOne = FVector(0, 0, 600);
	LocationTwo = FVector(0, -600, 600);
	LocationThree = FVector(0, 600, 600);
	LocationFour = FVector(-300, 0, 600);
	LocationFive = FVector(-400, -600, 600);

	MyBox = FBox(FVector(0, 0, 0), FVector(200, 200, 200));
	ProcMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMesh"));
	//RootComponent = ProcMesh;
}
void AMyActor::CreateMesh()
{
	Vertices.Add(FVector(-50, 0, 50));
	Vertices.Add(FVector(-50, 0, -50));
	Vertices.Add(FVector(50, 0, 50));
	Vertices.Add(FVector(50, 0, -50));

	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(0, 1));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(1, 1));

	//Triangle1
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);

	//Triangle2
	Triangles.Add(2);
	Triangles.Add(1);
	Triangles.Add(3);

	ProcMesh->CreateMeshSection(0, Vertices, Triangles, TArray<FVector>(), UVs, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	if (Material)
	{
		ProcMesh->SetMaterial(0, Material);
	}
}


void AMyActor::Execute_RenderThread()
{
	check(IsInRenderingThread());

	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();
	auto ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

	FRDGBuilder GraphBuilder(RHICmdList);
	TShaderMapRef<FSimplePS> PixelShader(ShaderMap);
	//TShaderMapRef<FSimpleVS> VertexShader(ShaderMap);
	check(PixelShader.IsValid());
	FSimplePS::FParameters* PassParameters = GraphBuilder.AllocParameters<FSimplePS::FParameters>();
	//FCopyTextureParameters* ParametersCopy = GraphBuilder.AllocParameters<FCopyTextureParameters>();


	FRDGTexture* Texture = GraphBuilder.CreateTexture(FRDGTextureDesc::Create2D(
		FIntPoint(RenderTarget2D->SizeX, RenderTarget2D->SizeY),
		PF_FloatRGBA,
		FClearValueBinding(),
		TexCreate_ShaderResource | TexCreate_RenderTargetable,
		1,
		1), TEXT("TextureS"));
	/*
	Texture->Desc.Extent;
	Texture->Desc.ClearValue;
	Texture->Desc.Format;
	Texture->Desc.NumMips;
	Texture->Desc.NumSamples;
	Texture->Desc.Flags;
	//Texture->Desc.DebugName;
	*/

	FTexture2DRHIRef RHIRef = RenderTarget2D->GetRenderTargetResource()->GetRenderTargetTexture();
	check(RHIRef.IsValid());
	//FSceneRenderTargetItem Item;
	//Item.ShaderResourceTexture = RHIRef;
	//Item.TargetableTexture = RHIRef;
	/*FPooledRenderTargetDesc PooledRenderTargetDesc = FPooledRenderTargetDesc::Create2DDesc(FIntPoint(RenderTarget2D->SizeX, RenderTarget2D->SizeY),
		PF_FloatRGBA, FClearValueBinding(),
		TexCreate_ShaderResource | TexCreate_RenderTargetable,
		TexCreate_ShaderResource |TexCreate_RenderTargetable | TexCreate_Dynamic, false);*/
	
	//GRenderTargetPool.CreateUntrackedElement(PooledRenderTargetDesc, PooledRenderTarget, Item);
	
	//TRefCountPtr<IPooledRenderTarget> PooledRenderTarget = CreateRenderTarget(RHIRef, TEXT("rhMap"));
	//FRDGTexture* OutputTexture = GraphBuilder.RegisterExternalTexture(PooledRenderTarget, TEXT("DepthMap"));
	auto OutputTexture = RegisterExternalTexture(GraphBuilder, RenderTarget2D->GetRenderTargetResource()->GetTexture2DRHI(), TEXT("RT_CameraView_GraphBuilder"));
	check(OutputTexture);
	PassParameters->StartColor = FVector4f(0.8, 0.6, 0.9, 0.3);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTexture, ERenderTargetLoadAction::EClear);

	const FIntPoint DestTextureSize(OutputTexture->Desc.Extent.X , OutputTexture->Desc.Extent.Y );

	UE_LOG(LogTemp, Warning, TEXT("OutputTexture->Desc.Extent.X,Y %d %d"), OutputTexture->Desc.Extent.X , OutputTexture->Desc.Extent.Y);
	FPixelShaderUtils::AddFullscreenPass(
		GraphBuilder,
		ShaderMap,
		RDG_EVENT_NAME("Generat Texture example"),
		PixelShader,
		PassParameters,
		FIntRect(FIntPoint::ZeroValue, DestTextureSize));


	
	//ParametersCopy->Input = Texture;
	//ParametersCopy->Output = OutputTexture;
	
	FRHICopyTextureInfo CopyInfo;
	/*GraphBuilder.AddPass(
		RDG_EVENT_NAME("CopyTexture"),
		ParametersCopy,
		ERDGPassFlags::Copy,
		[Texture, OutputTexture, CopyInfo](FRHICommandList& RHICmdList)
		{
			RHICmdList.CopyTexture(Texture->GetRHI(), OutputTexture->GetRHI(), CopyInfo);
		});
	*/


	//AddCopyTexturePass(GraphBuilder, Texture, OutputTexture, CopyInfo);

	UE_LOG(LogTemp, Warning, TEXT(" Executes this"));
	

	GraphBuilder.Execute();
	//RHICmdList.EndRenderPass();
	//FRHICopyTextureInfo CopyInfo;
	//RHICmdList.CopyTexture(Texture->GetRHI(), RenderTarget2D->GetResource()->GetTexture2DArrayRHI(), CopyInfo);
	

}



// Called when the game starts or when spawned
void AMyActor::BeginPlay()
{
	Super::BeginPlay();

	
	

	CreateMesh();



}



void AMyActor::RunTest_draw()
{
	FlushRenderingCommands();

	ENQUEUE_RENDER_COMMAND(FRayTracingTestbed)(
		[&](FRHICommandListImmediate& RHICmdList)
		{
			Execute_RenderThread();
		}
	);

	FlushRenderingCommands();

}

// Called every frame
void AMyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT(" Tick actor"));
	FVector NewLocation = GetActorLocation();
	FRotator NewRotation = GetActorRotation();
	float RunningTime = GetGameTimeSinceCreation();
	float DeltaHeight = (FMath::Sin(RunningTime + DeltaTime) - FMath::Sin(RunningTime));
	NewLocation.Z += DeltaHeight * 20.0f;       //Scale our height by a factor of 20
	float DeltaRotation = DeltaTime * 20.0f;    //Rotate by 20 degrees per second
	NewRotation.Yaw += DeltaRotation;
	SetActorLocationAndRotation(NewLocation, NewRotation);
	float StartAngle = 0;
	float EndAngle = 0;
	float CaptureFov = 0;
	uint32 NumRaysH = 1;
	this->RunTest_draw();
	//UE_LOG(LogTemp, Warning, TEXT("The boolean value is %s"), (update ? TEXT("true") : TEXT("false")));

}

