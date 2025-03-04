<template>
    <div class="rooms">
        <h1 style="color: white;">Your rooms</h1>
        <ul>
            <li v-for="room in rooms" :key="room.id" @click="selectRoom(room)">
                {{ room.name }}
            </li>
        </ul>
    </div>
</template>


<script setup>
    import { ref, onMounted } from 'vue'
    import { useUserStore } from '@/store/user'
    import { useRoomStore } from '@/store/room'
    import { useApiStore } from '@/store/api'
    import axios from 'axios'

    const rooms = ref([])
    const userStore = useUserStore()
    const roomStore = useRoomStore()
    const apiStore = useApiStore()

    const getRoom = async (room_id) => {
        try {
            const response = await axios.get(`${apiStore.rest_url}/rooms/${room_id}`, {
                headers: {
                    Authorization: `Bearer ${localStorage.getItem('token')}`
                }
            })

            if (response.status == 200) {
                const room_data = response.data
                room_data.room_id = room_id
                rooms.value.push(room_data)
            }
        }
        catch (error) {
            console.error('Failed to fetch room: ', error)
        }
    }

    const selectRoom = (room) => {
        roomStore.setRoom(room)
    }
onMounted(() => {
        rooms.value = []
        userStore.room_ids.forEach(room_id => {
            getRoom(room_id)
        })
    })
</script>


<style scoped>

.rooms {
    display: flex;
    flex-direction: column;
    height: 80vh;
    padding-left: 20px;
    background-color: var(--foreground);
}

h1 {
    margin: 0;
    padding-bottom: 10px;
}

ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    color: var(--vue-green);
    cursor: pointer;
    text-decoration: underline;
    font-size: 14pt;
    overflow-y: auto;
}

ul:hover {
    text-decoration: none;
}

</style>
