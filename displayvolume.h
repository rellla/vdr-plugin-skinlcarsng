// --- cLCARSNGDisplayVolume -----------------------------------------------

class cLCARSNGDisplayVolume : public cSkinDisplayVolume {
private:
  cOsd *osd;
  cLCARSNGVolumeBox *volumeBox;
public:
  cLCARSNGDisplayVolume(void);
  virtual ~cLCARSNGDisplayVolume();
  virtual void SetVolume(int Current, int Total, bool Mute);
  virtual void Flush(void);
  };
