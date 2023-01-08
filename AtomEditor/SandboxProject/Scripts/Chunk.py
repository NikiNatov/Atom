import Atom

class Chunk(Atom.Entity):
    Distance: float

    def __init__(self, entityID: int):
        super().__init__(entityID)
        self.Distance = 5.0

    def on_create(self):
        Atom.Log.info("Chunk::OnCreate")

    def on_update(self, ts: Atom.Timestep):
        Atom.Log.info("Chunk::OnUpdate")

