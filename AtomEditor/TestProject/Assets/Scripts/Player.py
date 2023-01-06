import Atom

class Player(Atom.Entity):
    MoveSpeed: float
    JumpForce: float
    RotationSpeed: float
    _RigidBody: Atom.RigidbodyComponent

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.MoveSpeed = 50.0
        self.JumpForce = 10.0
        self.RotationSpeed = 5.0
        self._RigidBody = Atom.RigidbodyComponent()

    def on_create(self):
        Atom.Log.info("Player::OnCreate")
        
        width = 5.0
        height = 5.0
        
        vertices = Atom.VertexList([Atom.Vertex()] * 4)
        vertices[0].position = Atom.Vec3(-width / 2.0, 0.0, -height / 2.0)
        vertices[0].uv = Atom.Vec2(0.0, 0.0) * 20.0
        vertices[0].normal = Atom.Vec3(0.0, 1.0, 0.0)
        vertices[0].tangent = Atom.Vec3(1.0, 0.0, 0.0)
        vertices[0].bitangent = Atom.Vec3(0.0, 0.0, 1.0)
        
        vertices[1].position = Atom.Vec3(width / 2.0, 0.0, -height / 2.0)
        vertices[1].uv = Atom.Vec2(1.0, 0.0) * 20.0
        vertices[1].normal = Atom.Vec3(0.0, 1.0, 0.0)
        vertices[1].tangent = Atom.Vec3(1.0, 0.0, 0.0)
        vertices[1].bitangent = Atom.Vec3(0.0, 0.0, 1.0)
        
        vertices[2].position = Atom.Vec3(width / 2.0, 0.0, height / 2.0)
        vertices[2].uv = Atom.Vec2(1.0, 1.0) * 20.0
        vertices[2].normal = Atom.Vec3(0.0, 1.0, 0.0)
        vertices[2].tangent = Atom.Vec3(1.0, 0.0, 0.0)
        vertices[2].bitangent = Atom.Vec3(0.0, 0.0, 1.0)
        
        vertices[3].position = Atom.Vec3(-width / 2.0, 0.0, height / 2.0)
        vertices[3].uv = Atom.Vec2(0.0, 1.0) * 20.0
        vertices[3].normal = Atom.Vec3(0.0, 1.0, 0.0)
        vertices[3].tangent = Atom.Vec3(1.0, 0.0, 0.0)
        vertices[3].bitangent = Atom.Vec3(0.0, 0.0, 1.0)

        indices = Atom.Uint32List([ 0, 3, 2, 2, 1, 0 ])

        submeshes = Atom.SubmeshList([Atom.Submesh()])
        submeshes[0].start_vertex = 0
        submeshes[0].vertex_count = len(vertices)
        submeshes[0].start_index = 0
        submeshes[0].index_count = len(indices)
        submeshes[0].material_index = 0

        mesh = Atom.Mesh()
        mesh.vertices = vertices
        mesh.indices = indices
        mesh.submeshes = submeshes
        mesh.set_material(0, Atom.Material.find("Materials/08 - Default.atmmat"))
        mesh.update_gpu_data(True)

        self._RigidBody = self.get_rigidbody_component()
        self.get_mesh_component().mesh = mesh

    def on_update(self, ts: Atom.Timestep):
        velocity = Atom.Vec3(0.0)

        if Atom.Input.is_key_pressed(Atom.Key.W):
            velocity.z = -1.0
        elif Atom.Input.is_key_pressed(Atom.Key.S):
            velocity.z = 1.0

        if Atom.Input.is_key_pressed(Atom.Key.A):
            velocity.x = -1.0
        elif Atom.Input.is_key_pressed(Atom.Key.D):
            velocity.x = 1.0

        if Atom.Input.is_key_pressed(Atom.Key.Space):
            self._RigidBody.add_impulse(Atom.Vec3(0.0, self.JumpForce, 0.0), True)

        velocity = Atom.Vec3(velocity.x * self.MoveSpeed, self._RigidBody.velocity.y, velocity.z * self.MoveSpeed)
        self._RigidBody.velocity = velocity
