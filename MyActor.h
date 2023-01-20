	virtual void BeginPlay() override;
	UMaterialInterface* Material;
public:	
	// Called every frame
	bool ComputePointCloud_RenderThread();
	bool RunTest();
	void RunTest_draw();
	void Execute_RenderThread();
	virtual void Tick(float DeltaTime) override;

	// declare location variables
	UPROPERTY(EditAnywhere, Category = "Locations")
		FVector LocationOne;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FVector LocationTwo;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FVector LocationThree;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FVector LocationFour;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FVector LocationFive;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FMatrix CircleMatrix;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FBox MyBox;

	UPROPERTY(EditAnywhere, Category = "Locations")
		FTransform MyTransform;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;
	void CreateMesh();
	UTextureRenderTarget2D* RenderTarget2D;
	UTexture2D* Texture2D;
	//USceneCaptureComponent2D* sceneCapture;
	FRHITexture2D* FRHITexture_x;


};
