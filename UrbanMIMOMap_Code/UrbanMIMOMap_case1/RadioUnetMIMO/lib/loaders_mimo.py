import os
import warnings

import cv2  # Need OpenCV for resizing
import numpy as np
import torch
from skimage import io
from torch.utils.data import Dataset
from torchvision import transforms

warnings.filterwarnings("ignore")


class RadioUNet_c_mimo(Dataset):
    """
    Dataset for radio map prediction with MIMO configuration.
    
    Loads 256x256 building maps and transmitter locations, upscales to 512x512 inputs,
    and provides 512x512 radio gain maps as targets.
    
    Args:
        maps_inds: Dataset indices. Default generates shuffled indices 0-349.
        phase: Dataset split ("train", "val", "test", or custom).
        ind1, ind2: Custom range bounds for non-standard phases.
        dir_dataset: Base dataset directory path.
        numTx: Number of transmitters per map (default: 40).
        thresh: Processing threshold (default: 0.0).
        transform: Data transformation (default: ToTensor).
    """

    def __init__(self, maps_inds=np.zeros(1), phase="train",
                 ind1=0, ind2=0,
                 dir_dataset=r"\UrbanMIMOMap",
                 numTx=40,
                 thresh=0.0,
                 transform=transforms.ToTensor()
                 ):
        """Initialize RadioUNet MIMO dataset."""
        # Generate map indices if not provided
        if maps_inds.size == 1:
            self.maps_inds = np.arange(0, 350, 1, dtype=np.int16)
            np.random.seed(42)
            np.random.shuffle(self.maps_inds)
        else:
            self.maps_inds = maps_inds
            
        # Set dataset split boundaries
        if phase == "train":
            self.ind1, self.ind2 = 0, 249
        elif phase == "val":
            self.ind1, self.ind2 = 250, 299
        elif phase == "test":
            self.ind1, self.ind2 = 300, 349
        else:  # custom range
            self.ind1, self.ind2 = ind1, ind2
            
        # Store configuration parameters
        self.dir_dataset = dir_dataset
        self.numTx = numTx
        self.thresh = thresh
        self.transform = transform
        
        # Define data directories
        self.dir_buildings_256 = os.path.join(self.dir_dataset, "env_diffH_withcars")
        self.dir_Tx_256 = os.path.join(self.dir_dataset, "antennas")
        self.dir_gain_512 = os.path.join(self.dir_dataset, "gain")
        
        # Image dimensions
        self.height_256, self.width_256 = 256, 256
        self.height_512, self.width_512 = 512, 512

    def __len__(self):
        """Return total number of samples: maps × transmitters × azimuth angles."""
        return (self.ind2 - self.ind1 + 1) * self.numTx * 3

    def __getitem__(self, idx):
        """
        Retrieve dataset sample.
        
        Args:
            idx: Sample index.
            
        Returns:
            tuple: (input_tensor, target_tensor, sample_name)
                - input_tensor: 4-channel 512x512 input (buildings, Tx, sin_azi, cos_azi)
                - target_tensor: 1-channel 512x512 radio gain map
                - sample_name: Identifier string
        """
        # Calculate sample indices
        idx_azi = np.floor(idx % 3).astype(int)
        azi = idx_azi * 60  # Azimuth angle: 0°, 60°, 120°
        
        idx_map = np.floor(idx / self.numTx / 3).astype(int)
        idx_tx = (idx - idx_map * self.numTx * 3) // 3
        dataset_map_ind = self.maps_inds[idx_map + self.ind1]

        # Construct filenames
        name_map_256 = f"{dataset_map_ind}.png"
        name_tx_256 = f"{dataset_map_ind}_{idx_tx}.png"
        name_gain_512 = f"{dataset_map_ind}_{idx_tx}_{azi}.png"

        # Load 256x256 source data
        img_path_buildings_256 = os.path.join(self.dir_buildings_256, name_map_256)
        image_buildings_256 = np.asarray(io.imread(img_path_buildings_256)).astype(np.float32) / 255.0
        if image_buildings_256.ndim == 3:
            image_buildings_256 = image_buildings_256[:, :, 0]

        img_path_Tx_256 = os.path.join(self.dir_Tx_256, name_tx_256)
        image_Tx_256 = np.asarray(io.imread(img_path_Tx_256))
        if image_Tx_256.ndim == 3:
            image_Tx_256 = image_Tx_256[:, :, 0]

        # Load 512x512 target data
        img_path_gain_512 = os.path.join(self.dir_gain_512, f"rm_rss_map_{dataset_map_ind}", name_gain_512)
        image_gain_512 = np.asarray(io.imread(img_path_gain_512)).astype(np.float32) / 255.0
        if image_gain_512.ndim == 3:
            image_gain_512 = image_gain_512[:, :, 0]

        # Process 256x256 sources to 512x512 inputs
        # Find transmitter locations
        tx_y_256_coords, tx_x_256_coords = np.where(image_Tx_256 > 0)
        
        if len(tx_y_256_coords) == 0:
            print(f"Warning: No Tx pixel found in {img_path_Tx_256}")

        # Map coordinates to 512x512
        tx_y_512_coords = tx_y_256_coords * 2
        tx_x_512_coords = tx_x_256_coords * 2

        # Upsample buildings map
        image_buildings_512 = cv2.resize(
            image_buildings_256, (self.width_512, self.height_512), 
            interpolation=cv2.INTER_NEAREST)

        # Create 512x512 transmitter map
        image_Tx_512 = np.zeros((self.height_512, self.width_512), dtype=np.float32)
        if len(tx_y_512_coords) > 0:
            image_Tx_512[tx_y_512_coords, tx_x_512_coords] = 1.0

        # Create azimuth angle maps
        azi_rad = np.deg2rad(azi)
        sin_azi_val, cos_azi_val = np.sin(azi_rad), np.cos(azi_rad)

        image_sin_azi_512 = np.zeros((self.height_512, self.width_512), dtype=np.float32)
        image_cos_azi_512 = np.zeros((self.height_512, self.width_512), dtype=np.float32)

        if len(tx_y_512_coords) > 0:
            image_sin_azi_512[tx_y_512_coords, tx_x_512_coords] = sin_azi_val
            image_cos_azi_512[tx_y_512_coords, tx_x_512_coords] = cos_azi_val

        # Stack input channels
        inputs = np.stack([image_buildings_512, image_Tx_512,
                          image_sin_azi_512, image_cos_azi_512], axis=2)

        # Apply transformations
        if self.transform:
            inputs_tensor = self.transform(inputs).type(torch.float32)
            image_gain_512_expanded = np.expand_dims(image_gain_512, axis=2)
            image_gain_tensor = self.transform(image_gain_512_expanded).type(torch.float32)
            
        name_gain_for_check = f"{dataset_map_ind}_{idx_tx}_{azi}"
        return [inputs_tensor, image_gain_tensor, name_gain_for_check]


# Example usage
if __name__ == '__main__':
    base_dir = r"/mnt/mydisk/hgjia/data/UrbanMIMOMap/"
    dataset = RadioUNet_c_mimo(phase="train", dir_dataset=base_dir)
    print(f"Dataset size: {len(dataset)}")

    # Test single sample
    inputs, gains, name = dataset[0]
    print(f"Input shape: {inputs.shape}")
    print(f"Target shape: {gains.shape}")
    print(f"Input data type: {inputs.dtype}")
    print(f"Target data type: {gains.dtype}")
    print(f"Input value range: min={inputs.min().item()}, max={inputs.max().item()}")
    print(f"Target value range: min={gains.min().item()}, max={gains.max().item()}")

    # Test DataLoader
    try:
        dl = torch.utils.data.DataLoader(dataset, batch_size=2, shuffle=True)
        for i, data in enumerate(dl):
            inputs_batch, gains_batch, name = data
            print(f"Batch {i}: Input shape: {inputs_batch.shape}, Target shape: {gains_batch.shape}")
            break
    except Exception as e:
        print(f"DataLoader test error: {e}")
