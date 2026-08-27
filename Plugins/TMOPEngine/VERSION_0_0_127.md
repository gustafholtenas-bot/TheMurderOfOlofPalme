# TMOPEngine 0.0.127

## Automatic traffic-light click emitters

- Finds traffic-light actors by actor, component or Static Mesh asset name.
- Default matching supports `trafikljus`, `trafficlight` and `traffic_light`.
- Attaches one `TRAFFIC_LIGHT_CLICK` loop per matching actor.
- Starts nearby emitters and fades them out when the listener leaves the area.
- Periodically discovers streamed-in traffic-light meshes.
- SoundLibrary `bLoop` is now honoured for Sound Wave assets.
