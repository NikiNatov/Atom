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

        positions: list = [
            Atom.Vec3(-width / 2.0, 0.0, -height / 2.0),
            Atom.Vec3( width / 2.0, 0.0, -height / 2.0),
            Atom.Vec3( width / 2.0, 0.0,  height / 2.0),
            Atom.Vec3(-width / 2.0, 0.0,  height / 2.0)
        ]

        uvs: list = [
            Atom.Vec2(0.0, 0.0) * 20.0,
            Atom.Vec2(1.0, 0.0) * 20.0,
            Atom.Vec2(1.0, 1.0) * 20.0,
            Atom.Vec2(0.0, 1.0) * 20.0
        ]

        normals: list = [
            Atom.Vec3(0.0, 1.0, 0.0),
            Atom.Vec3(0.0, 1.0, 0.0),
            Atom.Vec3(0.0, 1.0, 0.0),
            Atom.Vec3(0.0, 1.0, 0.0)
        ]

        tangents: list = [
            Atom.Vec3(1.0, 0.0, 0.0),
            Atom.Vec3(1.0, 0.0, 0.0),
            Atom.Vec3(1.0, 0.0, 0.0),
            Atom.Vec3(1.0, 0.0, 0.0)
        ]

        bitangents: list = [
            Atom.Vec3(0.0, 0.0, 1.0),
            Atom.Vec3(0.0, 0.0, 1.0),
            Atom.Vec3(0.0, 0.0, 1.0),
            Atom.Vec3(0.0, 0.0, 1.0)
        ]

        indices: list = [ 0, 3, 2, 2, 1, 0 ]

        submeshes: list = [Atom.Submesh()]
        submeshes[0].start_vertex = 0
        submeshes[0].vertex_count = len(positions)
        submeshes[0].start_index = 0
        submeshes[0].index_count = len(indices)
        submeshes[0].material_index = 0

        mesh: Atom.Mesh = Atom.Mesh()
        mesh.set_positions(positions)
        mesh.set_uvs(uvs)
        mesh.set_normals(normals)
        mesh.set_tangents(tangents)
        mesh.set_bitangents(bitangents)
        mesh.set_indices(indices)
        mesh.set_submeshes(submeshes)
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
