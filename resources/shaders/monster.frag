uniform sampler2D texture;
uniform vec2 size;
uniform vec2 offset;

const float pixelation = 2.0f;

void main() {
    vec2 coord = gl_TexCoord[0].xy + offset / size; 

    vec2 roundCoord = vec2(floor(coord * size * pixelation + 0.5) / size / pixelation) - offset / size;
    vec4 pixel = texture2D(texture, roundCoord);

    gl_FragColor = gl_Color * pixel;
}
