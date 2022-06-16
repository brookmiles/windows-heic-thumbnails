#include <shlwapi.h>
#include <thumbcache.h> // For IThumbnailProvider.
#include <new>

#include <libheif/heif.h>

#include "log.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Pathcch.lib")

// this thumbnail provider implements IInitializeWithStream to enable being hosted
// in an isolated process for robustness

class CHEICThumbProvider : public IInitializeWithStream,
    public IThumbnailProvider
{
public:
    CHEICThumbProvider() : _cRef(1), _pStream(NULL)
    {
    }

    virtual ~CHEICThumbProvider()
    {
        if (_pStream)
        {
            _pStream->Release();
        }

    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CHEICThumbProvider, IInitializeWithStream),
            QITABENT(CHEICThumbProvider, IThumbnailProvider),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream* pStream, DWORD grfMode);

    // IThumbnailProvider
    IFACEMETHODIMP GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);

private:

    long _cRef;
    IStream* _pStream;     // provided during initialization.
};

HRESULT CHEICThumbProvider_CreateInstance(REFIID riid, void** ppv)
{
    CHEICThumbProvider* pNew = new (std::nothrow) CHEICThumbProvider();
    HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pNew->QueryInterface(riid, ppv);
        pNew->Release();
    }
    return hr;
}

// IInitializeWithStream
IFACEMETHODIMP CHEICThumbProvider::Initialize(IStream* pStream, DWORD)
{
    HRESULT hr = E_UNEXPECTED;  // can only be inited once
    if (_pStream == NULL)
    {
        // take a reference to the stream if we have not been inited yet
        hr = pStream->QueryInterface(&_pStream);
    }
    return hr;
}

//HRESULT CreateTestBitmap(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
//{
//    HRESULT hr = E_FAIL;
//
//    UINT nWidth = cx;
//    UINT nHeight = cx;
//
//    BITMAPINFO bmi = {};
//    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
//    bmi.bmiHeader.biWidth = nWidth;
//    bmi.bmiHeader.biHeight = -static_cast<LONG>(nHeight);
//    bmi.bmiHeader.biPlanes = 1;
//    bmi.bmiHeader.biBitCount = 32;
//    bmi.bmiHeader.biCompression = BI_RGB;
//
//    UINT stride = ((((bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount) + 31) & ~31) >> 3);
//
//    BYTE* pBits = 0;
//    HBITMAP hbmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&pBits), NULL, 0);
//    hr = hbmp ? S_OK : E_OUTOFMEMORY;
//    if (SUCCEEDED(hr))
//    {
//        for (UINT y = 0; y < nHeight; ++y)
//        {
//            DWORD* row = reinterpret_cast<DWORD*>(&pBits[y * stride]);
//            for (UINT x = 0; x < nWidth; ++x)
//            {
//                // 0xAARRGGBB
//
//                row[x] = 0xFF000000 | 
//                    ((y % 256) << 16) |
//                    (x % 256);
//            }
//        }
//        *phbmp = hbmp;
//        *pdwAlpha = WTSAT_ARGB;
//    }
//
//    return hr;
//}

HRESULT CreateDIBFromData(HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha, const uint8_t* src_data, int thumbnail_width, int thumbnail_height, int src_stride)
{
    HRESULT hr = E_FAIL;

    UINT nWidth = thumbnail_width;
    UINT nHeight = thumbnail_height;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = nWidth;
    bmi.bmiHeader.biHeight = -static_cast<LONG>(nHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    UINT dest_stride = ((((bmi.bmiHeader.biWidth * bmi.bmiHeader.biBitCount) + 31) & ~31) >> 3);

    BYTE* dest_data = 0;
    HBITMAP hbmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(&dest_data), NULL, 0);
    hr = hbmp ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        for (UINT y = 0; y < nHeight; ++y)
        {
            // 0xAARRGGBB
            DWORD* dest_row = reinterpret_cast<DWORD*>(&dest_data[y * dest_stride]);

            // 0xAABBGGRR
            const DWORD* src_row = reinterpret_cast<const DWORD*>(&src_data[y * src_stride]);

            for (UINT x = 0; x < nWidth; ++x)
            {
                dest_row[x] = 
                    (src_row[x] & 0xFF000000) | 
                    ((src_row[x] & 0x00FF0000) >> 16) |
                    (src_row[x] & 0x0000FF00) | 
                    ((src_row[x] & 0x000000FF) << 16);
            }
        }
        *phbmp = hbmp;
        *pdwAlpha = WTSAT_ARGB;
    }
    else
    {
        Log_WriteFmt(LOG_ERROR, L"CreateDIBSection (%u x %u) failed: 0x%08x", nWidth, nHeight, GetLastError());
    }

    return hr;

}
// IThumbnailProvider
IFACEMETHODIMP CHEICThumbProvider::GetThumbnail(UINT requested_size, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
    Log_WriteFmt(LOG_TRACE, L"CHEICThumbProvider::GetThumbnail(%u)", requested_size);

    ULARGE_INTEGER ulSize;

    HRESULT final_hr = E_FAIL;

    HRESULT hr = IStream_Size(_pStream, &ulSize);
    if (SUCCEEDED(hr))
    {
        Log_WriteFmt(LOG_DEBUG, L"stream size {%u, %u}", ulSize.HighPart, ulSize.LowPart);

        if (ulSize.HighPart == 0)
        {
            void* ptr = LocalAlloc(LPTR, ulSize.LowPart);
            if (ptr)
            {
                ULONG ulRead = 0;
                hr = _pStream->Read(ptr, ulSize.LowPart, &ulRead);
                if (SUCCEEDED(hr))
                {
                    Log_WriteFmt(LOG_DEBUG, L"stream read: %u", ulRead);

                    heif_context* ctx = heif_context_alloc();
                    heif_context_read_from_memory_without_copy(ctx, ptr, ulSize.LowPart, nullptr);

                    // --- get primary image
                    struct heif_image_handle* image_handle = NULL;
                    heif_error err = heif_context_get_primary_image_handle(ctx, &image_handle);
                    if (err.code) 
                    {
                        Log_WriteFmt(LOG_WARNING, L"Could not read HEIF image: %S", err.message);
                    }
                    else
                    {
                        heif_item_id thumbnail_ID;
                        int nThumbnails = heif_image_handle_get_list_of_thumbnail_IDs(image_handle, &thumbnail_ID, 1);
                        if (nThumbnails > 0) 
                        {
                            Log_WriteFmt(LOG_DEBUG, L"Image has thumbnails: %i", nThumbnails);

                            struct heif_image_handle* thumbnail_handle;
                            err = heif_image_handle_get_thumbnail(image_handle, thumbnail_ID, &thumbnail_handle);
                            if (err.code) 
                            {
                                Log_WriteFmt(LOG_WARNING, L"Could not read HEIF thumbnail: %S", err.message);
                            }
                            else
                            {
                                // replace image handle with thumbnail handle
                                heif_image_handle_release(image_handle);
                                image_handle = thumbnail_handle;
                            }
                        }
                    }

                    if (image_handle)
                    {
                        struct heif_decoding_options* decode_options = heif_decoding_options_alloc();
                        decode_options->convert_hdr_to_8bit = true;

                        int bit_depth = 8;

                        struct heif_image* image = NULL;
                        err = heif_decode_image(image_handle, &image, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, decode_options);
                        if (err.code) 
                        {
                            Log_WriteFmt(LOG_WARNING, L"Could not decode HEIF image: %S", err.message);
                        }
                        else
                        {
                            int input_width = heif_image_handle_get_width(image_handle);
                            int input_height = heif_image_handle_get_height(image_handle);

                            Log_WriteFmt(LOG_DEBUG, L"HEIF image/thumb size: %i x %i", input_width, input_height);

                            int thumbnail_width = input_width;
                            int thumbnail_height = input_height;

                            bool scale_error = false;
                            if (input_width > (int)requested_size || input_height > (int)requested_size)
                            {
                                int scaled_w = input_width;
                                int scaled_h = input_height;

                                if (input_width > input_height)
                                {
                                    scaled_h = input_height * requested_size / input_width;
                                    scaled_w = requested_size;
                                }
                                else // (input_width <= input_height)
                                {
                                    scaled_w = input_width * requested_size / input_height;
                                    scaled_h = requested_size;
                                }

                                Log_WriteFmt(LOG_INFO, L"scaling image (%i, %i) to (%i, %i)", input_width, input_height, scaled_w, scaled_h);

                                struct heif_image* scaled_image = NULL;
                                err = heif_image_scale_image(image, &scaled_image, scaled_w, scaled_h, NULL);
                                if (err.code) 
                                {
                                    Log_WriteFmt(LOG_WARNING, L"Could not scale HEIF image: %S", err.message);
                                    scale_error = true;
                                }
                                else
                                {
                                    heif_image_release(image);
                                    image = scaled_image;

                                    thumbnail_width = scaled_w;
                                    thumbnail_height = scaled_h;
                                }
                            }

                            if (!scale_error)
                            {
                                int stride;
                                const uint8_t* data = heif_image_get_plane_readonly(image, heif_channel_interleaved, &stride);

                                final_hr = CreateDIBFromData(phbmp, pdwAlpha, data, thumbnail_width, thumbnail_height, stride);
                            }

                            heif_image_release(image);
                        }

                        heif_image_handle_release(image_handle);
                    }

                    heif_context_free(ctx);
                }

                LocalFree(ptr);
            }
        }
    }

    //hr = CreateTestBitmap(cx, phbmp, pdwAlpha);

    if (FAILED(hr))
        return hr;

    return final_hr;
}
