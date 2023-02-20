import Atom

class Camera(Atom.Entity):
    Player: Atom.Entity
    Distance: float

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.Player = Atom.Entity(0)
        self.Distance = 5.0

    def on_create(self):
        Atom.Log.info("Camera::OnCreate")

    def on_update(self, ts: Atom.Timestep):
        if self.Player.is_valid():
            self.transform.translation = Atom.Vec3(self.Player.transform.translation.x, self.Player.transform.translation.y + 2.0, self.Player.transform.translation.z + self.Distance)

