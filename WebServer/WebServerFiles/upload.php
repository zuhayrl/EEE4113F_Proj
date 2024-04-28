<?php
if(isset($_FILES['file'])) {
    $files = $_FILES['file'];
    $upload_dir = 'uploads/';

    foreach ($files['name'] as $key => $value) {
        $filename = $files['name'][$key];
        $tmp_name = $files['tmp_name'][$key];
        $upload_file = $upload_dir . basename($filename);

        if (move_uploaded_file($tmp_name, $upload_file)) {
            echo "File '$filename' uploaded successfully.\n";
        } else {
            echo "Error uploading file '$filename'.\n";
        }
    }
}
?>

<form method="POST" enctype="multipart/form-data">
    <input type="file" name="file[]" multiple>
    <input type="submit" value="Upload">
</form>
