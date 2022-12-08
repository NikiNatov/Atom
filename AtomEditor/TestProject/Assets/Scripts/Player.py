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
        self._RigidBody = self.get_rigidbody_component()

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

        #if Atom.Input.is_key_pressed(Atom.Key.Q):
        #    velocity.x = -1.0
        #elif Atom.Input.is_key_pressed(Atom.Key.E):
        #    velocity.x = 1.0

        if Atom.Input.is_key_pressed(Atom.Key.Space):
            self._RigidBody.add_impulse(Atom.Vec3(0.0, self.JumpForce, 0.0), True)

        velocity = Atom.Vec3(velocity.x * self.MoveSpeed, self._RigidBody.velocity.y, velocity.z * self.MoveSpeed)
        self._RigidBody.velocity = velocity
