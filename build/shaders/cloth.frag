#version 330 core
in vec3 FragPos, Normal;
out vec4 FragColor;
uniform vec3 lightPos, viewPos, objectColor, lightColor;
void main() {
    vec3 norm = normalize(Normal);
    if (!gl_FrontFacing) norm = -norm;
    vec3 ambient = 0.2 * lightColor;
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = max(dot(norm, lightDir), 0.0) * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 specular = 0.5 * pow(max(dot(viewDir, reflectDir), 0.0), 32.0) * lightColor;
    FragColor = vec4((ambient + diffuse + specular) * objectColor, 1.0);
}
