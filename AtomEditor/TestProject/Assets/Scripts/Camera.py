import Atom

class Camera(Atom.Entity):
    Player: Atom.Entity
    MoveSpeed: int

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.Player = Atom.Entity(0)
        self.MoveSpeed = 11

    def on_create(self):
        Atom.Log.info("Camera::OnCreate")

    def on_update(self, ts: Atom.Timestep):
        if self.Player.is_valid():
            self.translation = Atom.Vec3(self.Player.translation.x, self.Player.translation.y + 2.0, self.Player.translation.z + 5.0)

