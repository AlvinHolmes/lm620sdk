# 例程描述

- 本例程基于官方大评估板进行测试，需自行接入扬声器。
- ~~本例程默认已在 "cfg/cust_option.lua" 中将 audio 组件，CODEC: codec_es8311，PA: pa_aw87390 的配置打开，对应的是大评估板上的器件。~~
- 本例程默认已在 "cfg/cust_option.lua" 中将 audio 组件，CODEC: codec_pt8311 的配置打开，对应的是 E837N 上的器件。
- 本例程包含了 纯 PCM 与 MP3、AMR、WAV、OPUS 文件类型的数组数据的播放测试，但仅 纯 PCM、MP3、WAV 进行了测试。需在工程管理文件 "xmake.lua" 中，根据实际需求，开启对应的配置项。

  - 如需测试播放 MP3 文件类型的数组数据，则在 "xmake.lua" 中开启测试类型为 MP3 文件类型的数组数据；开启 MP3 编解码器的配置项，如下：

    ```lua
      -- set_config("test_type", "pcm_raw")
      -- set_config("test_type", "amr_file_data")
      set_config("test_type", "mp3_file_data")
      -- set_config("test_type", "wav_file_data")
      -- set_config("test_type", "opus_file_data")

      -- audio codec
      -- set_config("amr",true)
      set_config("mp3", true)
      -- set_config("pcm_u_a", true)
      -- set_config("opus", true)
      -- set_config("ogg", true)

    ```
