import { DivIcon, LatLng } from "leaflet";
import { CustomMarker } from "./custommarker";

export class CoalitionAreaHandle extends CustomMarker {
    constructor(latlng: LatLng) {
        super(latlng, {interactive: true, draggable: true});
    }

    createIcon() {
        this.setIcon(new DivIcon({
            iconSize: [52, 52],
            iconAnchor: [26, 26],
            className: "leaflet-target-marker",
        }));
        var el = document.createElement("div");
        el.classList.add("ol-target-icon");
        this.getElement()?.appendChild(el);
    }
}