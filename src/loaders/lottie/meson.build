source_file = [
   'tvgLottieLoader.h',
   'tvgLottieLoader.cpp',
   'lottieanimation/lottieanimation.cpp',
   'lottieanimation/lottieitem_capi.cpp',
   'lottieanimation/lottieitem.cpp',
   'lottieanimation/lottiekeypath.cpp',
   'lottieanimation/lottieloader.cpp',
   'lottieanimation/lottiemodel.cpp',
   'lottieanimation/lottieparser.cpp',
   'lottieanimation/lottieproxymodel.cpp',
   'lottieanimation/vector/varenaalloc.cpp',
   'lottieanimation/vector/vbezier.cpp',
   'lottieanimation/vector/vbitmap.cpp',
   'lottieanimation/vector/vbrush.cpp',
   'lottieanimation/vector/vdasher.cpp',
   'lottieanimation/vector/vdebug.cpp',
   'lottieanimation/vector/vdrawable.cpp',
   'lottieanimation/vector/vdrawhelper_common.cpp',
   'lottieanimation/vector/vdrawhelper.cpp',
   'lottieanimation/vector/vdrawhelper_neon.cpp',
   'lottieanimation/vector/vdrawhelper_sse2.cpp',
   'lottieanimation/vector/velapsedtimer.cpp',
   'lottieanimation/vector/vimageloader.cpp',
   'lottieanimation/vector/vinterpolator.cpp',
   'lottieanimation/vector/vmatrix.cpp',
   'lottieanimation/vector/vpainter.cpp',
   'lottieanimation/vector/vpath.cpp',
   'lottieanimation/vector/vpathmesure.cpp',
   'lottieanimation/vector/vrect.cpp',
   'lottieanimation/vector/vraster.cpp',
   'lottieanimation/vector/vrle.cpp',

   'lottieanimation/vector/freetype/v_ft_math.cpp',
   'lottieanimation/vector/freetype/v_ft_raster.cpp',
   'lottieanimation/vector/freetype/v_ft_stroker.cpp',
   'lottieanimation/vector/stb/stb_image.cpp',
]

#Temporary
rlottie_dep = dependency('rlottie', required: false)

subloader_dep += [declare_dependency(
    include_directories : include_directories('.'),
    dependencies : rlottie_dep,
    sources : source_file
)]

