package main

import (
	"fmt"
	"image"
	_ "image/jpeg"
	_ "image/png"
	"math"
	"os"
	"path/filepath"
	"text/template"

	"github.com/g3n/engine/loader/obj"
	"github.com/g3n/engine/math32"
)

type vec3 struct {
	x, y, z float64
}

func (v vec3) Sub(o vec3) vec3 {
	return vec3{
		v.x - o.x,
		v.y - o.y,
		v.z - o.z,
	}
}

func (v vec3) String() string {
	return fmt.Sprintf("{%f, %f, %f}", v.x, v.y, v.z)
}

type vec2 struct {
	x, y float64
}

func (v vec2) String() string {
	return fmt.Sprintf("{%f, %f}", v.x, v.y)
}

type face struct {
	v1, v2, v3    int
	uv1, uv2, uv3 int
	mat_index     int
}

func (f face) String() string {
	return fmt.Sprintf("{%d, %d, %d, %d, %d, %d, %d}", f.v1, f.v2, f.v3, f.uv1, f.uv2, f.uv3, f.mat_index)
}

type material struct {
	ambientCol, diffuseCol, specularCol vec3
	Ns                                  float32
	owns_kd                             bool
}

func (m material) String() string {
	return fmt.Sprintf("{.ambient = %v, .diffuse = %v, .specular = %v, .Ns = %f, .owns_kd = %v}", m.ambientCol, m.diffuseCol, m.specularCol, m.Ns, m.owns_kd)
}

type Model struct {
	Name           string
	Verts          []vec3
	Normals        []vec3
	UVs            []vec2
	Faces          []face
	Materials      []material
	Material_map   map[string]int // = map[string]int{}
	Material_count int            // = 0

	Kd_width  int
	Kd_height int
	Kd_map    []uint32
}

func NewModel() Model {
	return Model{
		Name:           "",
		Verts:          []vec3{},
		Faces:          []face{},
		Materials:      []material{},
		Material_map:   map[string]int{},
		Material_count: 0,
	}
}

func (m *Model) get_mat_index(name string) int {
	return m.Material_map[name]
}

func (m *Model) add_material(name string) {
	m.Material_map[name] = m.Material_count
	m.Material_count++
}

func ColorToV3(c math32.Color) vec3 {
	return vec3{
		x: float64(c.R),
		y: float64(c.G),
		z: float64(c.B),
	}
}

func TriNormal(v1, v2, v3 vec3) vec3 {
	var A vec3 = v2.Sub(v1)
	var B vec3 = v3.Sub(v1)
	var Nx float64 = A.y*B.z - A.z*B.y
	var Ny float64 = A.z*B.x - A.x*B.z
	var Nz float64 = A.x*B.y - A.y*B.x
	return vec3{Nx, Ny, Nz}
}

var defualt_material = material{
	ambientCol:  vec3{1.0, 1.0, 1.0},
	diffuseCol:  vec3{1.0, 1.0, 1.0},
	specularCol: vec3{1.0, 1.0, 1.0},
	Ns:          1,
}

func main() {
	if len(os.Args) < 3 {
		fmt.Println("usage: ./converter path/to/model.obj path/to/model.mtl")
		return
	}
	objName := os.Args[1]
	mtlName := os.Args[2]

	name := filepath.Base(objName[:len(objName)-len(filepath.Ext(objName))])
	fmt.Println("Name: ", name)

	mod := LoadModel(objName, mtlName, name)

	fmt.Println("BINARY")
	SaveBinary(mod)
	// SaveText(mod)

}

func AddColorUint32(n uint32, buf *[]byte) {
	b1 := uint8((n & 0xFF000000) >> 24)
	b2 := uint8((n & 0x00FF0000) >> 16)
	b3 := uint8((n & 0x0000FF00) >> 8)
	b4 := uint8((n & 0xFF))

	*buf = append((*buf), []byte{b3, b2, b1, b4}...)
}

func AddUint32(n uint32, buf *[]byte) {
	b1 := uint8((n & 0xFF000000) >> 24)
	b2 := uint8((n & 0x00FF0000) >> 16)
	b3 := uint8((n & 0x0000FF00) >> 8)
	b4 := uint8((n & 0xFF))

	*buf = append((*buf), []byte{b4, b3, b2, b1}...)
}

func uint8FromBool(b bool) uint8 {
	if b {
		return 1
	}
	return 0
}

func SaveBinary(mod Model) {
	f, err := os.Create("out.vobj")
	check(err)

	bytes_per_uint32 := 4
	bytes_per_vec3 := 3 * 4
	bytes_per_vec2 := 2 * 4
	bytes_per_face := 7 * bytes_per_uint32
	bytes_per_material := bytes_per_vec3*3 + 4 + 1

	bytes_needed := bytes_per_uint32*6 +
		bytes_per_vec3*len(mod.Verts) +
		bytes_per_vec2*len(mod.UVs) +
		bytes_per_face*len(mod.Faces) +
		bytes_per_material*len(mod.Materials) +
		bytes_per_uint32*mod.Kd_height*mod.Kd_width

	bys := make([]byte, 0, bytes_needed)

	AddUint32(uint32(len(mod.Materials)), &bys)
	AddUint32(uint32(len(mod.Verts)), &bys)
	AddUint32(uint32(len(mod.UVs)), &bys)
	AddUint32(uint32(len(mod.Faces)), &bys)

	AddUint32(uint32(mod.Kd_width), &bys)
	AddUint32(uint32(mod.Kd_height), &bys)

	for _, v := range mod.Verts {
		AddUint32(math.Float32bits(float32(v.x)), &bys)
		AddUint32(math.Float32bits(float32(v.y)), &bys)
		AddUint32(math.Float32bits(float32(v.z)), &bys)
	}

	fmt.Println("uv offset", len(bys))
	for _, uv := range mod.UVs {
		AddUint32(math.Float32bits(float32(uv.x)), &bys)
		AddUint32(math.Float32bits(float32(uv.y)), &bys)
	}

	for _, f := range mod.Faces {
		AddUint32(uint32(f.v1), &bys)
		AddUint32(uint32(f.v2), &bys)
		AddUint32(uint32(f.v3), &bys)

		AddUint32(uint32(f.uv1), &bys)
		AddUint32(uint32(f.uv2), &bys)
		AddUint32(uint32(f.uv3), &bys)

		AddUint32(uint32(f.mat_index), &bys)

	}

	for _, m := range mod.Materials {
		AddUint32(math.Float32bits(float32(m.ambientCol.x)), &bys)
		AddUint32(math.Float32bits(float32(m.ambientCol.y)), &bys)
		AddUint32(math.Float32bits(float32(m.ambientCol.z)), &bys)

		AddUint32(math.Float32bits(float32(m.diffuseCol.x)), &bys)
		AddUint32(math.Float32bits(float32(m.diffuseCol.y)), &bys)
		AddUint32(math.Float32bits(float32(m.diffuseCol.z)), &bys)

		AddUint32(math.Float32bits(float32(m.specularCol.x)), &bys)
		AddUint32(math.Float32bits(float32(m.specularCol.y)), &bys)
		AddUint32(math.Float32bits(float32(m.specularCol.z)), &bys)

		AddUint32(math.Float32bits(float32(m.Ns)), &bys)
		bys = append(bys, uint8FromBool(m.owns_kd))

	}

	for _, pix := range mod.Kd_map {
		AddColorUint32(pix, &bys)
	}

	fmt.Printf("Thought %d but was %d\n", bytes_needed, len(bys))

	_, err = f.Write(bys)
	check(err)

	err = f.Close()
	check(err)

}

func SaveText(mod Model) {

	// Make template

	header_f, err := os.Create("out.h")
	check(err)
	header_templ, err := template.ParseFiles("h_template.txt")
	check(err)
	err = header_templ.Execute(header_f, mod)
	check(err)
	header_f.Close()

	source_f, err := os.Create("out.cpp")
	check(err)
	source_templ, err := template.ParseFiles("cpp_template.txt")
	check(err)
	err = source_templ.Execute(source_f, mod)
	check(err)
	source_f.Close()

}

func LoadModel(objName, mtlName, name string) Model {

	path := filepath.Dir(objName)

	mod, err := obj.Decode(objName, mtlName)
	if err != nil {
		panic(err)
	}

	m1 := NewModel()
	m1.Name = name

	num_images := 0
	m1.Materials = make([]material, len(mod.Materials))
	// Decode materials
	for name, mat := range mod.Materials {
		m1.add_material(name)
		mat_index := m1.get_mat_index(name)
		m1.Materials[mat_index].ambientCol = ColorToV3(mat.Ambient)
		m1.Materials[mat_index].diffuseCol = ColorToV3(mat.Diffuse)
		m1.Materials[mat_index].specularCol = ColorToV3(mat.Specular)
		m1.Materials[mat_index].Ns = float32(mat.Shininess)
		//fmt.Printf("mat.Illum: %v\n", mat.Shininess)

		kd_filename := mat.MapKd
		if kd_filename == "" {
			continue
		}
		num_images++
		if num_images > 1 {
			fmt.Println("Can only handle one kd map per object :(. if only victor had programmed this")
			os.Exit(1)
		}
		// get kd map
		imgf, err := os.Open(path + "/" + filepath.Base(kd_filename))
		check(err)
		img, _, err := image.Decode(imgf)
		check(err)
		m1.Kd_height = img.Bounds().Dy()
		m1.Kd_width = img.Bounds().Dx()
		m1.Materials[mat_index].owns_kd = true

		fmt.Println("dimensions", img.Bounds().Dx(), "x", img.Bounds().Dy())

		m1.Kd_map = make([]uint32, m1.Kd_height*m1.Kd_width)

		for y := 0; y < img.Bounds().Dy(); y++ {
			for x := 0; x < img.Bounds().Dx(); x++ {
				r, g, b, a := img.At(x, y).RGBA()
				r /= 256
				g /= 256
				b /= 256
				a /= 256
				ri := uint8(r)
				gi := uint8(g)
				bi := uint8(b)
				m1.Kd_map[y*m1.Kd_width+x] = (0xFF000000) + (uint32(ri) << 16) + (uint32(gi) << 8) + uint32(bi)
			}
		}

	}
	if len(m1.Materials) == 0 {
		m1.Materials = []material{defualt_material}
	}

	mod_vs := mod.Vertices.ToFloat32()
	m1.Verts = make([]vec3, len(mod_vs)/3)
	// Decode vertices
	for i := 0; i < len(mod.Vertices); i += 3 {
		m1.Verts[i/3] = vec3{
			x: float64(mod_vs[i]),
			y: float64(mod_vs[i+1]),
			z: float64(mod_vs[i+2]),
		}
	}

	mod_uvs := mod.Uvs.ToFloat32()
	m1.UVs = make([]vec2, len(mod_uvs)/2)
	// Decode vertices
	for i := 0; i < len(mod.Uvs); i += 2 {
		m1.UVs[i/2] = vec2{
			x: float64(mod_uvs[i]),
			y: float64(mod_uvs[i+1]),
		}
	}

	m1.Faces = make([]face, len(mod.Objects[0].Faces))

	// Decode Faces
	for i := 0; i < len(mod.Objects[0].Faces); i++ {
		face := mod.Objects[0].Faces[i]
		if len(face.Vertices) != 3 {
			fmt.Println("TRIANGULATE YOUR MODEL FIRST - IF YOU DID THAT, YOU WOULDNT GET THIS ERROR")
			panic("TRIANGULATE")
		}
		m1.Faces[i].v1 = face.Vertices[0]
		m1.Faces[i].v2 = face.Vertices[1]
		m1.Faces[i].v3 = face.Vertices[2]

		m1.Faces[i].uv1 = face.Uvs[0]
		m1.Faces[i].uv2 = face.Uvs[1]
		m1.Faces[i].uv3 = face.Uvs[2]

		m1.Faces[i].mat_index = m1.get_mat_index(face.Material)
	}

	// calculate world normals
	m1.Normals = make([]vec3, len(m1.Faces))
	// calculated on robot

	fmt.Printf("Taking %d of %d objects\n", 1, len(mod.Objects))
	fmt.Printf("%d normals, %d faces %d verts", len(m1.Normals), len(m1.Faces), len(m1.Verts))
	if num_images < 1 {
		fmt.Printf(". No kd map\n")
	} else {
		fmt.Printf(". kd map: %dx%d\n", m1.Kd_width, m1.Kd_height)
	}
	return m1
}

func check(err error) {
	if err != nil {
		panic(err)
	}
}
