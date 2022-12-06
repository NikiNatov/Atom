import Atom

class Player(Atom.Entity):
    MoveSpeed: float
    Camera: Atom.Entity

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.MoveSpeed = 50.0
        self.Camera = Atom.Entity(0)

    def on_create(self):
        Atom.Log.info("Player::OnCreate")

    def on_update(self, ts: Atom.Timestep):
        velocity = Atom.Vec3(0.0)

        if Atom.Input.is_key_pressed(Atom.Key.W):
            velocity.y = 1.0
        elif Atom.Input.is_key_pressed(Atom.Key.S):
            velocity.y = -1.0

        if Atom.Input.is_key_pressed(Atom.Key.A):
            velocity.x = -1.0
        elif Atom.Input.is_key_pressed(Atom.Key.D):
            velocity.x = 1.0

        if Atom.Input.is_key_pressed(Atom.Key.Q):
            self.get_rigidbody_component().add_impulse(Atom.Vec3(0.0, 0.0, -1.0), True)

        velocity = Atom.Vec3(velocity.x * self.MoveSpeed, velocity.y * self.MoveSpeed, velocity.z * self.MoveSpeed)
        self.translation += velocity * ts.get_seconds()
