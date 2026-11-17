#version 410 core

in vec4 v_Color;
in vec2 v_TexCoords;
in float v_TexIndex;

out vec4 FragColor;

uniform sampler2D u_Textures[16];

void main() {
    int index = int(v_TexIndex);

    // Switch-case workaround for Intel iGPU driver compatibility.
    // These drivers require sampler array indices to be constant integral
    // expressions; a dynamic index like u_Textures[index] fails to link.
    vec4 texColor;
    switch (index) {
        case  0: texColor = texture(u_Textures[ 0], v_TexCoords); break;
        case  1: texColor = texture(u_Textures[ 1], v_TexCoords); break;
        case  2: texColor = texture(u_Textures[ 2], v_TexCoords); break;
        case  3: texColor = texture(u_Textures[ 3], v_TexCoords); break;
        case  4: texColor = texture(u_Textures[ 4], v_TexCoords); break;
        case  5: texColor = texture(u_Textures[ 5], v_TexCoords); break;
        case  6: texColor = texture(u_Textures[ 6], v_TexCoords); break;
        case  7: texColor = texture(u_Textures[ 7], v_TexCoords); break;
        case  8: texColor = texture(u_Textures[ 8], v_TexCoords); break;
        case  9: texColor = texture(u_Textures[ 9], v_TexCoords); break;
        case 10: texColor = texture(u_Textures[10], v_TexCoords); break;
        case 11: texColor = texture(u_Textures[11], v_TexCoords); break;
        case 12: texColor = texture(u_Textures[12], v_TexCoords); break;
        case 13: texColor = texture(u_Textures[13], v_TexCoords); break;
        case 14: texColor = texture(u_Textures[14], v_TexCoords); break;
        case 15: texColor = texture(u_Textures[15], v_TexCoords); break;
        default: texColor = vec4(1.0); break;
    }

    FragColor = texColor * v_Color;
}
