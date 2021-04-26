uniform sampler2D texture;
uniform vec2 size;
uniform vec2 offset;

const float pixelation = 1.0;
const float roundness = 2.0;

void main() {
    vec2 coord = gl_TexCoord[0].xy + offset / size; 

    vec2 roundCoord = vec2(floor(coord * size * pixelation + 0.5) / size / pixelation) - offset / size;
    vec4 pixel = texture2D(texture, roundCoord);

    if (pixel.a < 0.5) {
        vec4 surround = vec4(0.0, 0.0, 0.0, 0.0);
        float minDist = roundness;

        for (float dx = -roundness; dx <= roundness; dx += 1.0)
            for (float dy = -roundness; dy <= roundness; dy += 1.0) {
                vec2 surroundCoord = roundCoord + vec2(dx / size.x / pixelation, dy / size.y / pixelation);

                vec4 color = texture2D(texture, surroundCoord);

                float d = min(abs(dx), abs(dy));

                if (color.a > 0.5 && d < minDist) {
                    surround = color;
                    minDist = d;
                }
            }

        gl_FragColor = gl_Color * surround;
    }
    else
        gl_FragColor = gl_Color * pixel;
}
