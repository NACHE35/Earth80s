# Earth 80's — by NACHE

Synthé polyphonique (6 voix, VST3 / Standalone) inspiré du Juno-106, avec un filtre type Korg MS-20,
2 oscillateurs, 6 effets et **1000 presets**. Construit avec JUCE 8 (téléchargé automatiquement par CMake).
Le fichier produit s'appelle `Earth 80s.vst3` (sans apostrophe pour éviter les soucis de nom de fichier) ; le titre affiché est « EARTH 80'S ».

## Partie gauche : la Terre
- **Anneau du globe** : cutoff du filtre (le cycle jour/nuit suit l'ouverture du filtre)
- **Haut gauche** VOLUME · **haut droite** RESONANCE · **bas gauche** PORTAMENTO · **bas droite** TUNE (±100 cents)
- **Interrupteur LP / HP** sous le knob de résonance
- 6 voyants de voix, VU-mètre, on/off, menu de presets, double-clic sur le titre = menu caché

## Partie droite : 4 onglets
- **MASTER** : enveloppes ampli et filtre (ADSR), env amount, key track, drive, mode de voix (Poly / Mono / Unison), detune unison, spread stéréo, bend range, LFO (forme, vitesse, delay, pitch, filtre)
- **OSC 1** : forme d'onde (Saw, Pulse, Saw+Pulse, Tri, Sine) avec aperçu, octave, demi-ton, fine, largeur d'impulsion, PWM par LFO, niveau, sub-oscillateur, bruit
- **OSC 2** : idem + détune, **sync** sur l'osc 1, enveloppe → pitch
- **FX** (dans l'ordre du signal) : compresseur → phaser → flanger → chorus Juno (I / II / I+II) → delay ping-pong → reverb, chacun avec son interrupteur
- **Clavier à l'écran** (utile en Standalone), pitch bend, molette de modulation (CC1), pédale de sustain (CC64)

## Presets
1000 presets : 10 catégories × 5 styles × 20 presets (Bass, Lead, Pad, Keys, Brass, Strings, Pluck, Bell & Mallet, Atmosphere, FX & Sweeps ;
styles Classic 80's, Synthwave, Italo & Disco, Dark Cinema, Modern Retro). Menu : Catégorie > Style > Preset, flèches précédent/suivant,
sauvegarde de presets utilisateur (« Save current settings as... »).
Ils sont générés par le code (`Source/Presets.cpp`) : vous pouvez modifier les recettes de chaque catégorie / style.

## Compiler sur GitHub (Windows)
```
git init
git add .
git commit -m "Earth 80's 1.0"
git branch -M main
git remote add origin https://github.com/TON_PSEUDO/Earth80s.git
git push -u origin main
```
Onglet **Actions** → *Build Earth 80s (Windows)* → télécharger l'artefact `Earth80s-Windows`.
Copier `Earth 80s.vst3` dans `C:\Program Files\Common Files\VST3`, relancer le scan du DAW (c'est un **instrument** : à charger sur une piste MIDI/instrument).

## Personnalisation
- Police « Thriller » : placez `Thriller.ttf` dans `Resources/`.
- Titre / version / date / lien : constantes en haut de `Source/PluginEditor.cpp`.
- Paramètres : table unique dans `Source/Params.h`.
