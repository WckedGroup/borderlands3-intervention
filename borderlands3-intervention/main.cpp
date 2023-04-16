#include "pch.h"

static void* PostRenderHookAddress;

void Initialize()
{
	DWORD_PTR baseAddress = (DWORD_PTR)GetModuleHandleA(nullptr);

	MODULEINFO moduleInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)baseAddress, &moduleInfo, sizeof(moduleInfo));

	auto TNameEntryArrayAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x83\xEC\x28\x48\x8B\x05\x00\x00\x00\x00\x48\x85\xC0\x75\x00\xB9\x00\x00\x00\x00\x48\x89\x5C\x24\x20\xE8",
		"xxxxxxx????xxxx?x????xxxxxx",
		0x7
	);

	auto FChunkedFixedUObjectArrayAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x00\x00\x48\x8D\x00\x00",
		"xxx????xx??xx??",
		0x3
	);

	auto UEngineAddress = FindPointerPattern
	(
		baseAddress,
		moduleInfo.SizeOfImage,
		"\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x0F\x84\x00\x00\x00\x00\x48\x8B\x89",
		"xxx????xxxxx????xxx",
		0x3
	);

	UObject::GObjects = (FChunkedFixedUObjectArray*)(FChunkedFixedUObjectArrayAddress);
	FName::GNames = *(TNameEntryArray**)(TNameEntryArrayAddress);
	GEngine = *(UEngine**)(UEngineAddress);
	UCanvas::roboto = UObject::FindObject<UFont>("Font Roboto.Roboto");
	//UCanvas::tinyRoboto = UObject::FindObject<UFont>("Font RobotoTiny.RobotoTiny");
	//UCanvas::droidSansMono = UObject::FindObject<UFont>("Font DroidSansMono.DroidSansMono");
	//UCanvas::oakBody = UObject::FindObject<UFont>("Font OAK_BODY.OAK_BODY");
	//UCanvas::posteramaRegular = UObject::FindObject<UFont>("Font PosteramaText-Regular_Font.PosteramaText-Regular_Font");
	UCanvas::robotoDistanceField = UObject::FindObject<UFont>("Font RobotoDistanceField.RobotoDistanceField");
	UCanvas::defaultFont = UCanvas::roboto;
}

void HookPostRenderInitialize()
{
	auto viewport = GEngine->GameViewport;
	if (!viewport) return;

	auto vtable = viewport->VTable;
	if (!vtable) return;

	auto PostRenderHookAddress = vtable + 0x63;
	PostRenderOriginal = reinterpret_cast<PostRenderOriginal_t>(PostRenderHookAddress[0]);

	UKismetSystemLibrary::Init();

	UKismetMathLibrary::Init();

	UKismetInputLibrary::Init();

	UTestLibrary::Init();

	BuildMainMenu();

	Input::Initialize();

	auto hook = &PostRenderHook;
	PatchMem(PostRenderHookAddress, &hook, 8);
}

void CheatRemove()
{
	PatchMem(PostRenderHookAddress, &PostRenderOriginal, 8);
}