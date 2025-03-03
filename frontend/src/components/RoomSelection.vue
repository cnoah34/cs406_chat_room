<template>
    <div>
        <h1>Your rooms</h1>
        <ul>
            <li v-for-'room in rooms'>
                {{ room.name }}
            </li>
        </ul>

    </div>
</template>


<script setup>
    import { ref } from 'vue'
    import axios from 'axios'

    const rooms = ref([])

    const getRooms = () => {
        //const reponse = await axios.get('http://localhost:8080/rooms)
    }

</script>


<style scoped>

</style>
