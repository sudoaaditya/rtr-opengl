//Headers!
#define UNICODE
#include<windows.h>
#include<stdio.h>
#include<d3d11.h>
#include<d3dcompiler.h>
#include"../__XNAMath/xnamath.h"
#include"../__TextureTools/WICTextureLoader.h"

using namespace DirectX;

#pragma warning(disable: 4838)

//Linking
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "../__TextureTools/DirectXTK.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//CallBack
LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

//GLobal Vars!.
FILE *fptr = NULL;
const char *szLogFileName = "_3DTexturesLog.txt";

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;
bool gbActiveWindow = false;
bool gbFullScreen = false;
bool gbEscapePressed = false;

//D3D
float clearColor[4];
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
//Depth Stencil Buffer!.
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;

ID3D11VertexShader *gpVertexShader = NULL;
ID3D11PixelShader *gpPixelShader = NULL;
ID3D11InputLayout *gpInputLayout = NULL;
ID3D11Buffer *gpConstantBuffer = NULL;
//For Culling: Rasterizer State.
ID3D11RasterizerState *gpRasterizerState = NULL;
//Data Buffers
ID3D11Buffer *gpVertexBufferCubePos = NULL;
ID3D11Buffer *gpVertexBufferCubeTex = NULL;
ID3D11Buffer *gpVertexBufferPyramidPos = NULL;
ID3D11Buffer *gpVertexBufferPyramidTex = NULL;
//ShaderResourceView & SamplerState
ID3D11ShaderResourceView *gpSRVPyramidTex = NULL;
ID3D11SamplerState *gpSamplerStatePyramidTex = NULL;
ID3D11ShaderResourceView *gpSRVCubeTex = NULL;
ID3D11SamplerState *gpSamplerStateCubeTex = NULL;

float fRotAngleCube = 0.0f;
float fRotAnglePyramid = 0.0f;

//Uniforms Mapping Here!.
struct CBUFFER {
    XMMATRIX WorldViewProjectionMatrix;
};

XMMATRIX perspectiveProjMat;

//winMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //Functions!;
    HRESULT initialize(void);
    void display(void);
    void uninitialize(void);
    void update(void);

    //Vars!.
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    bool bDone = false;
    TCHAR szClassName[] = TEXT("Direct3D11");

    //File.
    if(fopen_s(&fptr, szLogFileName, "w") != 0) {
        MessageBox(NULL, TEXT("Cannot Create Log File"), TEXT("Err_Msg"), MB_OK);
        exit(0);
    }
    else {
        fprintf_s(fptr, "Log File Created Successfully!!..\n\n");
        fclose(fptr);
    }

    //initialize WNDCLASSEX 
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground = RGB(0, 0 , 0);
    wndclass.hInstance = hInstance;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szClassName;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindow(szClassName,
                    TEXT("D3D :: 3D Texturing"),
                    WS_OVERLAPPEDWINDOW,
                    100,
                    100,
                    WIN_WIDTH,
                    WIN_HEIGHT,
                    NULL,
                    NULL,
                    hInstance,
                    NULL);

    ghwnd = hwnd;

    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //Initialize D3D!.
    HRESULT hr;
    hr = initialize();
    if(FAILED(hr)) {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "Initialization Failed!.\n");
        fclose(fptr);
        DestroyWindow(hwnd);
        hwnd = NULL;
        ghwnd = NULL;
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "Initialization Successful!.\n");
        fclose(fptr);
    }

    //Message Loop!.
    while(bDone == false) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT){
                bDone = true;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if(gbActiveWindow == true) {
                if(gbEscapePressed == true){
                    bDone = true;
                }
            }
            display();
            update();
        }
    }

    //CleanUP
    uninitialize();

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
    //Functions!.
    HRESULT resize(int, int);
    void ToggleFullScreen();
    void uninitialize(void);

    //Vars!.
    HRESULT hr;

    switch(iMsg) {

        case WM_ACTIVATE:
            if(HIWORD(wParam) == 0) {
                gbActiveWindow = true;
            }
            else {
                gbActiveWindow = false;
            }
        break;

        case WM_ERASEBKGND:
            return(0);

        case WM_SIZE:
            if(gpID3D11DeviceContext) {
                hr = resize(LOWORD(lParam), HIWORD(lParam));
                if(FAILED(hr)){
                    fopen_s(&fptr, szLogFileName, "a+");
                    fprintf(fptr, "Resize Failed!.\n");
                    fclose(fptr);
                }
                else {
                    fopen_s(&fptr, szLogFileName, "a+");
                    fprintf(fptr, "Resize Succeeded!.\n");
                    fclose(fptr);
                }
            }
            break;
        case WM_KEYDOWN: 
            switch(wParam) {
                case VK_ESCAPE:
                    if(gbEscapePressed == false){
                        gbEscapePressed = true;
                    }
                    break;

                case 'F':
                case 'f':
                    ToggleFullScreen();
                default:
                    break;
            }
            break;

        case WM_CLOSE:
            uninitialize();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;    
    }
    return(DefWindowProc(hwnd, iMsg, wParam , lParam));
}

void ToggleFullScreen(void) {
    MONITORINFO mi;

    if(gbFullScreen == false){
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if(dwStyle & WS_OVERLAPPEDWINDOW){
            mi = { sizeof(MONITORINFO) };
            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, 
                            HWND_TOP,
                            mi.rcMonitor.left,
                            mi.rcMonitor.top,
                            mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom - mi.rcMonitor.top,
                            SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
            ShowCursor(FALSE);
            gbFullScreen = true;
    }
        else {
            SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
            SetWindowPlacement(ghwnd, &wpPrev);
            SetWindowPos(ghwnd, 
                        HWND_TOP, 
                        0, 0, 0, 0,
                        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

            ShowCursor(TRUE);
            gbFullScreen = false;
        }
}

HRESULT initialize(void) {
    //Funs & Vars
    HRESULT loadD3DTexture(const wchar_t*, ID3D11ShaderResourceView**);
    void uninitialize(void);
    HRESULT resize(int, int);   
    
    HRESULT hr;
    D3D_DRIVER_TYPE d3dDriverType;
    D3D_DRIVER_TYPE d3dDriverTypes[] = { D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_REFERENCE };
    D3D_FEATURE_LEVEL d3dFeatLvlReq = D3D_FEATURE_LEVEL_11_0;
    D3D_FEATURE_LEVEL d3dFeatLvlAcq = D3D_FEATURE_LEVEL_10_0;

    UINT createDeviceFlags = 0;
    UINT numDriverTypes = 0;
    UINT numFeatLvl = 1;    //We Only want One Level Required so here we have mentioned one.

    //CODE!.
    numDriverTypes = sizeof(d3dDriverTypes)/sizeof(d3dDriverTypes[0]);

    DXGI_SWAP_CHAIN_DESC swapChainDes;
    ZeroMemory((void *)&swapChainDes, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDes.BufferCount = 1;
    swapChainDes.BufferDesc.Width = WIN_WIDTH;
    swapChainDes.BufferDesc.Height = WIN_HEIGHT;
    swapChainDes.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDes.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDes.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDes.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDes.OutputWindow = ghwnd;
    swapChainDes.SampleDesc.Count = 1;
    swapChainDes.SampleDesc.Quality = 0;
    swapChainDes.Windowed = TRUE;

    for(UINT driverIndx = 0; driverIndx < numDriverTypes; driverIndx++) {
        d3dDriverType = d3dDriverTypes[driverIndx];
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,                   //Adapter
            d3dDriverType,          //Driver Type One From Array Each Time!
            NULL,                   //Software!
            createDeviceFlags,      // Creation Flags
            &d3dFeatLvlReq,         //Required Feature Levels!.
            numFeatLvl,             // Number of levels passed in last parameter
            D3D11_SDK_VERSION,      //SDk Version
            &swapChainDes,          //Swap Chain Description
            &gpIDXGISwapChain,      // Swap Chain! Rikama Bhanda
            &gpID3D11Device,        // Rikama bhanda for Device
            &d3dFeatLvlAcq,         // Acquired Feature Level.
            &gpID3D11DeviceContext);// Rikma Bhanda for Context!.

        if(SUCCEEDED(hr)){
            break;
        }
    }
    if(FAILED(hr)){
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11CreateDeviceAndSwapChain Failed!.\n");
        fclose(fptr);
        return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11CreateDeviceAndSwapChain Scuuessful!.\n And the Chosen Driver is Of\t");
        if(d3dDriverType == D3D_DRIVER_TYPE_HARDWARE){
            fprintf(fptr, "Hardware Type!\n");
        }
        else if(d3dDriverType == D3D_DRIVER_TYPE_WARP){
            fprintf(fptr, "Warp Type!\n");
        }
        else if(d3dDriverType == D3D_DRIVER_TYPE_REFERENCE){
            fprintf(fptr, "Software Type!\n");
        }
        else{
            fprintf(fptr, "Unknown Type!\n");
        }

        fprintf(fptr, "And the Supported Heighest Feature Level is\t");
        if(d3dFeatLvlAcq == D3D_FEATURE_LEVEL_11_0){
            fprintf(fptr, "11.0 \n");
        }
        else if(d3dFeatLvlAcq == D3D_FEATURE_LEVEL_10_1){
            fprintf(fptr, "10.1!\n");
        }
        else if(d3dFeatLvlAcq == D3D_FEATURE_LEVEL_10_0){
            fprintf(fptr, "10.0!\n");
        }
        else{
            fprintf(fptr, "UnKnown!\n");
        }
        fclose(fptr);
    }

    //Verter Shader!.
    const char* vertexShaderSourceCode =
        "cbuffer ConstantBuffer {"      \
        "   float4x4 worldViewProjMat;"     \
        "}" \
        "struct Vertex_Output {"    \
        "   float4 position : SV_POSITION; "   \
        "   float2 texCoord : TEXCOORD;" \
        "};" \
        "Vertex_Output main( float4 pos: POSITION, float2 texCoord : TEXCOORD ) {"   \
        "   Vertex_Output vertOut;" \
        "   vertOut.position = mul(worldViewProjMat, pos);" \
        "   vertOut.texCoord = texCoord;" \
        "   return(vertOut);"  \
        "}";

    ID3DBlob *pVertexShaderCode = NULL;
    ID3DBlob *pErrorBlob = NULL;

    hr = D3DCompile(
        vertexShaderSourceCode,
        lstrlenA(vertexShaderSourceCode) + 1,
        "VS",
        NULL,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        "vs_5_0",
        0,
        0,
        &pVertexShaderCode,
        &pErrorBlob);

    if(FAILED(hr)){
        if(pErrorBlob != NULL){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "D3DCompile Failed While Compiling Vertex Shader!.\n");
            fprintf(fptr, "Error:: %s", (char*)pErrorBlob->GetBufferPointer());
            fclose(fptr);
            pErrorBlob->Release();
            pErrorBlob = NULL;
            return(hr);
        }
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "Vertex Shader Compilation Successful!.\n");
        fclose(fptr);
    }

    hr = gpID3D11Device->CreateVertexShader(pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(), NULL, &gpVertexShader);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateVertexShader Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateVertexShader Successful!!.\n");
        fclose(fptr);
    }
    gpID3D11DeviceContext->VSSetShader(gpVertexShader, 0, 0);

    //Pixel Shader!
    const char* pixelShaderSourceCode = 
        "Texture2D myTex2D; " \
        "SamplerState texSamplerState;" \
        "float4 main(float4 position: SV_POSITION, float2 texCoord: TEXCOORD) : SV_TARGET {" \
        "   float4 color =  myTex2D.Sample(texSamplerState, texCoord);"  \
        "   return(color);" \
        "}";

        ID3DBlob *pPixelShaderCode = NULL;
        pErrorBlob = NULL;

        hr = D3DCompile(
            pixelShaderSourceCode,
            lstrlenA(pixelShaderSourceCode),
            "PS",
            NULL,
            D3D_COMPILE_STANDARD_FILE_INCLUDE,
            "main",
            "ps_5_0",
            0,
            0,
            &pPixelShaderCode,
            &pErrorBlob);

    if(FAILED(hr)){
        if(pErrorBlob != NULL){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "D3DCompile Failed While Compiling Fragment Shader!.\n");
            fprintf(fptr, "Error:: %s", (char*)pErrorBlob->GetBufferPointer());
            fclose(fptr);
            pErrorBlob->Release();
            pErrorBlob = NULL;
            return(hr);
        }
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "Fragment Shader Compilation Successful!.\n");
        fclose(fptr);
    }

    hr = gpID3D11Device->CreatePixelShader(pPixelShaderCode->GetBufferPointer(), pPixelShaderCode->GetBufferSize(), NULL, &gpPixelShader);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreatePixelShader Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreatePixelShader Successful!!.\n");
        fclose(fptr);
    }
    gpID3D11DeviceContext->PSSetShader(gpPixelShader, 0, 0);

    //Create Latout!. And SET IT.
    D3D11_INPUT_ELEMENT_DESC inputElementsDesc[2];
    ZeroMemory(&inputElementsDesc, sizeof(D3D11_INPUT_ELEMENT_DESC));
    inputElementsDesc[0].SemanticName = "POSITION";
    inputElementsDesc[0].SemanticIndex = 0;
    inputElementsDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementsDesc[0].InputSlot = 0;
    inputElementsDesc[0].AlignedByteOffset = 0;
    inputElementsDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputElementsDesc[0].InstanceDataStepRate = 0;

    inputElementsDesc[1].SemanticName = "TEXCOORD";
    inputElementsDesc[1].SemanticIndex = 0;
    inputElementsDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT; // 2 Coords
    inputElementsDesc[1].InputSlot = 1; // AMC_ATTRIBUTE_TEXCOORD = 1
    inputElementsDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;  //GL_UNPACK_ALIGNMENT : 4
    inputElementsDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    inputElementsDesc[1].InstanceDataStepRate = 0;

    hr = gpID3D11Device->CreateInputLayout(inputElementsDesc, 2, 
            pVertexShaderCode->GetBufferPointer(), 
            pVertexShaderCode->GetBufferSize(),
            &gpInputLayout);

    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateInputLayout Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateInputLayout Successful!!.\n");
        fclose(fptr);
    }

    gpID3D11DeviceContext->IASetInputLayout(gpInputLayout);
    //Release Shader Codes!
    pVertexShaderCode->Release();
    pPixelShaderCode->Release();
    pVertexShaderCode = NULL;
    pPixelShaderCode = NULL;

    float cubeVerties[] = {
        //TOP
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
        //Bottom
        1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
        //Front
        -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
        //Back
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 
        -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,
        //Left
        -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f ,
        -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        //Right
        1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  1.0f
    };

    float cubetexCoords[] = {
        //TOP
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
        //BOTTOM
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
        //FRONT
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
        //BACK
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
        //LEFT
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
        //Right
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 
        1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 
    };

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(cubeVerties);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //Create Buffer!
    hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpVertexBufferCubePos);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateBuffer Failed For Vertex Buffer For Cube Position!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateBuffer For Vertex Buffer For Cube Position Successful!!.\n");
        fclose(fptr);
    }
    //Copy Verteces into Above Buffer Using MAPPING
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    gpID3D11DeviceContext->Map(gpVertexBufferCubePos, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, cubeVerties, sizeof(cubeVerties));
    gpID3D11DeviceContext->Unmap(gpVertexBufferCubePos, 0);

    //Texture Cube!
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(cubetexCoords);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpVertexBufferCubeTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateBuffer Failed For Vertex Buffer For Cube Texture!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateBuffer For Vertex Buffer For Cube Texture Successful!!.\n");
        fclose(fptr);
    }
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    gpID3D11DeviceContext->Map(gpVertexBufferCubeTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, cubetexCoords, sizeof(cubetexCoords));
    gpID3D11DeviceContext->Unmap(gpVertexBufferCubeTex, 0);


    float pyramidVertices[] = {
        //Front
        0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        //Right
        0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        //Back
        0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        //Left
        0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 
    };

    float pyramidTexCoords[] = {
        //Fornt 
        0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        //Right
        0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 
        //Back
        0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 
        //Left
        0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
    };

    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(pyramidVertices);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //Create Buffer!
    hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpVertexBufferPyramidPos);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateBuffer Failed For Vertex Buffer For Pyramid Position!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateBuffer For Vertex Buffer For Pyramid Position Successful!!.\n");
        fclose(fptr);
    }
    //Copy Verteces into Abover Buffer Using MAPPING
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    gpID3D11DeviceContext->Map(gpVertexBufferPyramidPos, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, pyramidVertices, sizeof(pyramidVertices));
    gpID3D11DeviceContext->Unmap(gpVertexBufferPyramidPos, 0);

    //Pyramid COlor
    ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
    vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth = sizeof(float) * _ARRAYSIZE(pyramidTexCoords);
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    //Create Buffer!
    hr = gpID3D11Device->CreateBuffer(&vertexBufferDesc, NULL, &gpVertexBufferPyramidTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateBuffer Failed For Vertex Buffer For Pyramid Texture!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateBuffer For Vertex Buffer For Pyramid Texture Successful!!.\n");
        fclose(fptr);
    }
    //Copy Verteces into Abover Buffer Using MAPPING
    ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
    gpID3D11DeviceContext->Map(gpVertexBufferPyramidTex, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, pyramidTexCoords, sizeof(pyramidTexCoords));
    gpID3D11DeviceContext->Unmap(gpVertexBufferPyramidTex, 0);


    //Constant Buffer!.
    D3D11_BUFFER_DESC constantBUfferDesc;
    ZeroMemory(&constantBUfferDesc, sizeof(D3D11_BUFFER_DESC));
    constantBUfferDesc.Usage = D3D11_USAGE_DEFAULT;
    constantBUfferDesc.ByteWidth = sizeof(CBUFFER);
    constantBUfferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    
    hr = gpID3D11Device->CreateBuffer(&constantBUfferDesc, NULL, &gpConstantBuffer);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateBuffer Failed For Constant Buffer!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateBuffer For Constant Buffer Successful!!.\n");
        fclose(fptr);
    }
    gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpConstantBuffer);

    //Rasterizer State.
    D3D11_RASTERIZER_DESC rasterStateDesc;
    ZeroMemory(&rasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterStateDesc.AntialiasedLineEnable = FALSE;
    rasterStateDesc.CullMode = D3D11_CULL_NONE;
    rasterStateDesc.DepthBias = 0;
    rasterStateDesc.DepthBiasClamp = 0.0f;
    rasterStateDesc.DepthClipEnable = TRUE;
    rasterStateDesc.FillMode = D3D11_FILL_SOLID;
    rasterStateDesc.FrontCounterClockwise = FALSE;
    rasterStateDesc.MultisampleEnable = FALSE;
    rasterStateDesc.ScissorEnable = FALSE;
    rasterStateDesc.SlopeScaledDepthBias = 0.0f;
    hr = gpID3D11Device->CreateRasterizerState(&rasterStateDesc, &gpRasterizerState);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateRasterizerState Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateRasterizerState Successful!!.\n");
        fclose(fptr);
    }
    gpID3D11DeviceContext->RSSetState(gpRasterizerState);

    //Pyramid!.
    hr = loadD3DTexture(L"Stone.bmp", &gpSRVPyramidTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "loadD3DTexture for Pyramid Texture Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "loadD3DTexture for Pyramid Texture Successful!!.\n");
        fclose(fptr);
    }

    //Sampler Desc
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    //CreateSamplerState
    gpID3D11Device->CreateSamplerState(&samplerDesc, &gpSamplerStatePyramidTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateSamplerState for Pyramid Texture Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateSamplerState for Pyramid Texture Successful!!.\n");
        fclose(fptr);
    }

    //CUBE!.
    hr = loadD3DTexture(L"Vijay_Kundali.bmp", &gpSRVCubeTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "loadD3DTexture for Cube Texture Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "loadD3DTexture for Cube Texture Successful!!.\n");
        fclose(fptr);
    }

    //Sampler Desc
    ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    //CreateSamplerState
    gpID3D11Device->CreateSamplerState(&samplerDesc, &gpSamplerStateCubeTex);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "ID3D11Device::CreateSamplerState for Cube Texture Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "ID3D11Device::CreateSamplerState for Cube Texture Successful!!.\n");
        fclose(fptr);
    }

    // Clear Color!
    clearColor[0] = 0.0f;
    clearColor[1] = 0.0f;
    clearColor[2] = 0.0f;
    clearColor[3] = 1.0f;

    perspectiveProjMat = XMMatrixIdentity();

    hr = resize(WIN_WIDTH, WIN_HEIGHT);
    if(FAILED(hr)){
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "WarmUP resize Failed!.\n");
        fclose(fptr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "WarmUP Resize Sceesssful!.\n");
        fclose(fptr);
    }
    return(S_OK);
}

HRESULT loadD3DTexture(const wchar_t *texFileName, ID3D11ShaderResourceView **ppTexShaderRes) {
    HRESULT hr;

    //Create Shader.
    hr = CreateWICTextureFromFile(gpID3D11Device, gpID3D11DeviceContext, texFileName, NULL, ppTexShaderRes);
    if(FAILED(hr)){
            fopen_s(&fptr, szLogFileName, "a+");
            fprintf(fptr, "CreateWICTextureFromFile In loadD3DTexture Failed!!.\n");
            fclose(fptr);
            return(hr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "CreateWICTextureFromFile In loadD3DTexture Successful!!.\n");
        fclose(fptr);
    }
    return(hr);
}

HRESULT resize(int width, int height) {
    HRESULT hr = S_OK;

    //Free Size Dependent Resources!.
    if(gpID3D11DepthStencilView){
        gpID3D11DepthStencilView->Release();
        gpID3D11DepthStencilView = NULL;
    }

    if(gpID3D11RenderTargetView) {
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    //Resize Swap Chain Buffers Accordingly!.
    gpIDXGISwapChain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

    //Again get the bLack buffer from chain!
    ID3D11Texture2D *pID3D11Texture2DBlackBuffer;
    gpIDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pID3D11Texture2DBlackBuffer);

    //again Get Render Target View from d3d11 device using above black buffer!.
    hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2DBlackBuffer, NULL, &gpID3D11RenderTargetView);
    if(FAILED(hr)){
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11Device:  CreateRenderTargetView Failed!.\n");
        fclose(fptr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11Device:  CreateRenderTargetView Sceesssful!.\n");
        fclose(fptr);
    }

    pID3D11Texture2DBlackBuffer->Release();
    pID3D11Texture2DBlackBuffer = NULL;

    //Create Depth Stencil View!
    ID3D11Texture2D *pTexDepthBuffer = NULL;
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
    texDesc.Width = (UINT)width;
    texDesc.Height = (UINT)height;
    texDesc.ArraySize = 1,
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_D32_FLOAT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    gpID3D11Device->CreateTexture2D(&texDesc, NULL, &pTexDepthBuffer);
    
    //Depth Stencil View Desciption
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    hr = gpID3D11Device->CreateDepthStencilView(pTexDepthBuffer, &depthStencilViewDesc, &gpID3D11DepthStencilView);
    if(FAILED(hr)){
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11Device:  CreateDepthStencilView Failed!.\n");
        fclose(fptr);
    }
    else {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "D3D11Device:  CreateDepthStencilView Sceesssful!.\n");
        fclose(fptr);
    }

    pTexDepthBuffer->Release();
    pTexDepthBuffer = NULL;

    gpID3D11DeviceContext->OMSetRenderTargets(1, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

    //Set Viewport.
    D3D11_VIEWPORT d3dViewPort;
    d3dViewPort.TopLeftX = 0;
    d3dViewPort.TopLeftY = 0;
    d3dViewPort.Width = (float)width;
    d3dViewPort.Height = (float)height;
    d3dViewPort.MinDepth = 0.0f;
    d3dViewPort.MaxDepth = 1.0f;
    gpID3D11DeviceContext->RSSetViewports(1, &d3dViewPort);

    //Projection matrix!.
    perspectiveProjMat = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), ((float)width/(float)height), 0.1f, 100.0f);

    return(hr);
}

void display(void){
    //Clear The Render Targe With The color!.
    gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, clearColor);
    gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    //Set VertexBuffer TO Display!
    UINT iStride = sizeof(float) * 3;
    UINT iOffset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpVertexBufferCubePos, &iStride, &iOffset);
    iStride = sizeof(float) * 2;   //For U & V
    iOffset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpVertexBufferCubeTex, &iStride, &iOffset);

    //Binding PixelShader Resources!.
    gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpSRVCubeTex);
    gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpSamplerStateCubeTex);

    //Select Geometry Primitive!.
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Transformatuions!.
    XMMATRIX worldMat = XMMatrixIdentity();
    XMMATRIX viewMat = XMMatrixIdentity();
    XMMATRIX translateMat = XMMatrixIdentity();
    XMMATRIX worldViewProjMat = XMMatrixIdentity();
    XMMATRIX rotationMat = XMMatrixIdentity();
    XMMATRIX scaleMat = XMMatrixIdentity();

    translateMat = XMMatrixTranslation(2.0f, 0.0f, 6.0f);   //Postive Z Value Since it follows Left Hand Rule!
    rotationMat = XMMatrixRotationX(fRotAngleCube) * XMMatrixRotationY(fRotAngleCube) * XMMatrixRotationZ(fRotAngleCube);
    scaleMat = XMMatrixScaling(0.75f, 0.75f, 0.7f);
    worldMat *= scaleMat * rotationMat * translateMat;
    worldViewProjMat = worldMat * viewMat * perspectiveProjMat;
    
    //Load Data to  CBuffer and send over GPU
    CBUFFER cBuffer;
    cBuffer.WorldViewProjectionMatrix = worldViewProjMat;
    gpID3D11DeviceContext->UpdateSubresource(gpConstantBuffer, 0, NULL, &cBuffer, 0, 0);

    //Draw!!!..
    gpID3D11DeviceContext->Draw(6, 0);
    gpID3D11DeviceContext->Draw(6, 6);
    gpID3D11DeviceContext->Draw(6, 12);
    gpID3D11DeviceContext->Draw(6, 18);
    gpID3D11DeviceContext->Draw(6, 24);
    gpID3D11DeviceContext->Draw(6, 30);


    //Set VertexBuffer TO Display!
    iStride = sizeof(float) * 3;
    iOffset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpVertexBufferPyramidPos, &iStride, &iOffset);
    iStride = sizeof(float) * 2;   //For U & V
    iOffset = 0;
    gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpVertexBufferPyramidTex, &iStride, &iOffset);

    //Set PixelShader Resources
    gpID3D11DeviceContext->PSSetShaderResources(0, 1, &gpSRVPyramidTex);
    gpID3D11DeviceContext->PSSetSamplers(0, 1, &gpSamplerStatePyramidTex);

    //Select Geometry Primitive!.
    gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Pyramid
    worldMat = XMMatrixIdentity();
    viewMat = XMMatrixIdentity();
    translateMat = XMMatrixIdentity();
    rotationMat = XMMatrixIdentity();
    worldViewProjMat = XMMatrixIdentity();
    
    translateMat = XMMatrixTranslation(-2.0f, 0.0f, 6.0f);
    rotationMat = XMMatrixRotationY(-fRotAnglePyramid);
    worldMat *= rotationMat * translateMat;
    worldViewProjMat = worldMat * viewMat * perspectiveProjMat;
    cBuffer.WorldViewProjectionMatrix = worldViewProjMat;
    gpID3D11DeviceContext->UpdateSubresource(gpConstantBuffer, 0, NULL, &cBuffer, 0, 0);
    
    gpID3D11DeviceContext->Draw(12, 0);

    //Switch Betwwen Front & Back Buffer!
    gpIDXGISwapChain->Present(0, 0);
}

void update(void) {
    fRotAngleCube += 0.005f;
    if(fRotAngleCube >= 360.0f){
        fRotAngleCube = 0.0f;
    }

    fRotAnglePyramid += 0.005f;
    if(fRotAnglePyramid >= 360.0f){
        fRotAnglePyramid = 0.0f;
    }
}

void uninitialize(void) {

    //Code!.
    if(gpSamplerStateCubeTex) {
        gpSamplerStateCubeTex->Release();
        gpSamplerStateCubeTex = NULL;
    }

    if(gpSRVCubeTex) {
        gpSRVCubeTex->Release();
        gpSRVCubeTex = NULL;
    }

    if(gpSamplerStatePyramidTex) {
        gpSamplerStatePyramidTex->Release();
        gpSamplerStatePyramidTex = NULL;
    }

    if(gpSRVPyramidTex) {
        gpSRVPyramidTex->Release();
        gpSRVPyramidTex = NULL;
    }

    if(gpRasterizerState){
        gpRasterizerState->Release();
        gpRasterizerState = NULL;
    }

    if(gpConstantBuffer){
        gpConstantBuffer->Release();
        gpConstantBuffer = NULL;
    }

        if(gpInputLayout){
        gpInputLayout->Release();
        gpInputLayout = NULL;
    }

    if(gpVertexBufferCubePos){
        gpVertexBufferCubePos->Release();
        gpVertexBufferCubePos = NULL;
    }

    if(gpVertexBufferCubeTex){
        gpVertexBufferCubeTex->Release();
        gpVertexBufferCubeTex = NULL;
    }
    
    if(gpVertexBufferPyramidPos){
        gpVertexBufferPyramidPos->Release();
        gpVertexBufferPyramidPos = NULL;
    }

    if(gpVertexBufferPyramidTex){
        gpVertexBufferPyramidTex->Release();
        gpVertexBufferPyramidTex = NULL;
    }

        if(gpPixelShader){
        gpPixelShader->Release();
        gpPixelShader = NULL;
    }

        if(gpVertexShader){
        gpVertexShader->Release();
        gpVertexShader = NULL;
    }

    if(gpID3D11DepthStencilView){
        gpID3D11DepthStencilView->Release();
        gpID3D11DepthStencilView = NULL;
    }

    if(gpID3D11RenderTargetView){
        gpID3D11RenderTargetView->Release();
        gpID3D11RenderTargetView = NULL;
    }

    if(gpIDXGISwapChain) {
        gpIDXGISwapChain->Release();
        gpIDXGISwapChain = NULL;
    }

    if(gpID3D11DeviceContext) {
        gpID3D11DeviceContext->Release();
        gpID3D11DeviceContext = NULL;
    }

    if(gpID3D11Device) {
        gpID3D11Device->Release();
        gpID3D11Device = NULL;
    }

    if(fptr) {
        fopen_s(&fptr, szLogFileName, "a+");
        fprintf(fptr, "Unitialization Sceesssful!.\n");
        fprintf(fptr, "\nLog File Closed Sceesssful!.\n");
        fclose(fptr);
    }
}
