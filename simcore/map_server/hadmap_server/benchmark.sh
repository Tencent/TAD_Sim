#!/bin/bash

# create batch_xosc2simrec_urls.txt
# {\"id\":1,\"name\":\"1603893984.xosc\",\"category\":\"LOG_SIM\",\"bucket\":\"hadmap-only-1311246188\",\"key\":\"/scenes/multi-thread\",\"mapName\":\"map_name\",\"mapUrl\":\"maps/multi-thread/apollo_vectormap_G3_202307141425_re.xml\",\"mapVersion\":\"1.0\",\"egoType\":null}
# ...



cat batch_xosc2simrec_urls.txt | xargs -I {} -P 100 curl -s -X POST -H 'Content-Type: application/json' -d {} 'http://127.0.0.1:8080/scenario/xosc2simrec'
