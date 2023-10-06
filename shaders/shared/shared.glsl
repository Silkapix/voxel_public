#extension GL_EXT_fragment_shader_barycentric : require

vec4 wireframe(vec3 BaryCoord) {
  //* In pixels
  const f32 Thickness = 1.0;
  const f32 Falloff = 0.0;

	const vec3 dBaryCoordX = dFdxFine(BaryCoord);
	const vec3 dBaryCoordY = dFdyFine(BaryCoord);
	const vec3 dBaryCoord  = sqrt(dBaryCoordX*dBaryCoordX + dBaryCoordY*dBaryCoordY);

	const vec3 dFalloff   = dBaryCoord * Falloff;
	const vec3 dThickness = dBaryCoord * Thickness;

	const vec3 Remap = smoothstep(dThickness, dThickness + dFalloff, BaryCoord);
	const f32 closest_edge = min(min(Remap.x, Remap.y), Remap.z);

  if(closest_edge > 0.1) {
    discard;
  }
  return vec4(0.4, 1.0, 0.2, 1.0);
}